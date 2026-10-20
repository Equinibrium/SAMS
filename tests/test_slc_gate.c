#include <assert.h>
#include <stdio.h>

#include "slc_gate.h"

typedef struct { unsigned calls; bool succeed; } effect_state_t;

static bool effect_fn(void *context)
{
    effect_state_t *state = (effect_state_t *)context;
    ++state->calls;
    return state->succeed;
}

static slc_request_t request(void)
{
    return (slc_request_t){ .target_id = 1, .action_id = 2, .payload_digest = 3, .nonce = 99 };
}

static slc_authorization_t authorization(void)
{
    return (slc_authorization_t){
        .target_id = 1, .action_id = 2, .payload_digest = 3, .epoch = 4,
        .nonce = 99, .not_before = 10, .not_after = 20,
        .signature_valid = true, .issuer_authorized = true
    };
}

static void expect_reject(slc_result_t expected, slc_request_t req, slc_authorization_t auth, uint64_t now)
{
    slc_gate_t gate;
    effect_state_t effect = { .calls = 0, .succeed = true };
    slc_gate_init(&gate, 4);
    assert(slc_gate_execute(&gate, &req, &auth, now, effect_fn, &effect) == expected);
    assert(effect.calls == 0);
}

int main(void)
{
    slc_gate_t gate;
    effect_state_t effect = { .calls = 0, .succeed = true };
    slc_request_t req = request();
    slc_authorization_t auth = authorization();
    slc_gate_init(&gate, 4);

    assert(slc_gate_execute(&gate, &req, &auth, 15, effect_fn, &effect) == SLC_RESULT_COMMIT);
    assert(effect.calls == 1);

    assert(slc_gate_execute(&gate, &req, &auth, 15, effect_fn, &effect) == SLC_REJECT_REPLAY);
    assert(effect.calls == 1);

    req = request(); auth = authorization(); auth.signature_valid = false;
    expect_reject(SLC_REJECT_SIGNATURE, req, auth, 15);
    req = request(); auth = authorization(); auth.issuer_authorized = false;
    expect_reject(SLC_REJECT_ISSUER, req, auth, 15);
    req = request(); auth = authorization();
    expect_reject(SLC_REJECT_NOT_YET_VALID, req, auth, 9);
    req = request(); auth = authorization();
    expect_reject(SLC_REJECT_EXPIRED, req, auth, 21);
    req = request(); auth = authorization(); auth.epoch = 3;
    expect_reject(SLC_REJECT_EPOCH, req, auth, 15);
    req = request(); auth = authorization(); req.target_id = 8;
    expect_reject(SLC_REJECT_TARGET, req, auth, 15);
    req = request(); auth = authorization(); req.action_id = 8;
    expect_reject(SLC_REJECT_ACTION, req, auth, 15);
    req = request(); auth = authorization(); req.payload_digest = 8;
    expect_reject(SLC_REJECT_PAYLOAD, req, auth, 15);
    req = request(); auth = authorization(); req.nonce = 100;
    expect_reject(SLC_REJECT_NONCE_MISMATCH, req, auth, 15);

    req = request(); auth = authorization();
    effect = (effect_state_t){ .calls = 0, .succeed = false };
    slc_gate_init(&gate, 4);
    assert(slc_gate_execute(&gate, &req, &auth, 15, effect_fn, &effect) == SLC_REJECT_EFFECT_FAILED);
    assert(effect.calls == 1);
    assert(slc_gate_execute(&gate, &req, &auth, 15, effect_fn, &effect) == SLC_REJECT_REPLAY);
    assert(effect.calls == 1);

    assert(slc_gate_execute(NULL, &req, &auth, 15, effect_fn, &effect) == SLC_REJECT_BAD_ARGUMENT);
    puts("SLC gate tests passed");
    return 0;
}
