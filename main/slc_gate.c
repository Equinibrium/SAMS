#include "slc_gate.h"

#include <string.h>

void slc_gate_init(slc_gate_t *gate, uint32_t current_epoch)
{
    if (gate == NULL) {
        return;
    }

    memset(gate, 0, sizeof(*gate));
    gate->current_epoch = current_epoch;
}

static bool nonce_seen(const slc_gate_t *gate, uint64_t nonce)
{
    for (size_t i = 0; i < gate->replay_count; ++i) {
        if (gate->replay_nonces[i] == nonce) {
            return true;
        }
    }
    return false;
}

static void reserve_nonce(slc_gate_t *gate, uint64_t nonce)
{
    gate->replay_nonces[gate->replay_next] = nonce;
    gate->replay_next = (gate->replay_next + 1u) % SLC_REPLAY_CACHE_CAPACITY;
    if (gate->replay_count < SLC_REPLAY_CACHE_CAPACITY) {
        ++gate->replay_count;
    }
}

slc_result_t slc_gate_execute(
    slc_gate_t *gate,
    const slc_request_t *request,
    const slc_authorization_t *authorization,
    uint64_t now,
    slc_effect_fn effect,
    void *effect_context)
{
    if (gate == NULL || request == NULL || authorization == NULL || effect == NULL) {
        return SLC_REJECT_BAD_ARGUMENT;
    }
    if (!authorization->signature_valid) {
        return SLC_REJECT_SIGNATURE;
    }
    if (!authorization->issuer_authorized) {
        return SLC_REJECT_ISSUER;
    }
    if (now < authorization->not_before) {
        return SLC_REJECT_NOT_YET_VALID;
    }
    if (now > authorization->not_after) {
        return SLC_REJECT_EXPIRED;
    }
    if (authorization->epoch != gate->current_epoch) {
        return SLC_REJECT_EPOCH;
    }
    if (request->target_id != authorization->target_id) {
        return SLC_REJECT_TARGET;
    }
    if (request->action_id != authorization->action_id) {
        return SLC_REJECT_ACTION;
    }
    if (request->payload_digest != authorization->payload_digest) {
        return SLC_REJECT_PAYLOAD;
    }
    if (request->nonce != authorization->nonce) {
        return SLC_REJECT_NONCE_MISMATCH;
    }
    if (nonce_seen(gate, request->nonce)) {
        return SLC_REJECT_REPLAY;
    }

    /*
     * Reserve before calling the effect. If the callback fails, the nonce
     * remains consumed: this is intentionally fail-closed for this prototype.
     */
    reserve_nonce(gate, request->nonce);
    return effect(effect_context) ? SLC_RESULT_COMMIT : SLC_REJECT_EFFECT_FAILED;
}

const char *slc_result_name(slc_result_t result)
{
    switch (result) {
    case SLC_RESULT_COMMIT: return "COMMIT";
    case SLC_REJECT_BAD_ARGUMENT: return "REJECT_BAD_ARGUMENT";
    case SLC_REJECT_SIGNATURE: return "REJECT_SIGNATURE";
    case SLC_REJECT_ISSUER: return "REJECT_ISSUER";
    case SLC_REJECT_NOT_YET_VALID: return "REJECT_NOT_YET_VALID";
    case SLC_REJECT_EXPIRED: return "REJECT_EXPIRED";
    case SLC_REJECT_EPOCH: return "REJECT_EPOCH";
    case SLC_REJECT_TARGET: return "REJECT_TARGET";
    case SLC_REJECT_ACTION: return "REJECT_ACTION";
    case SLC_REJECT_PAYLOAD: return "REJECT_PAYLOAD";
    case SLC_REJECT_NONCE_MISMATCH: return "REJECT_NONCE_MISMATCH";
    case SLC_REJECT_REPLAY: return "REJECT_REPLAY";
    case SLC_REJECT_EFFECT_FAILED: return "REJECT_EFFECT_FAILED";
    default: return "REJECT_UNKNOWN";
    }
}
