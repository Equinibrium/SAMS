#ifndef SLC_GATE_H
#define SLC_GATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SLC_REPLAY_CACHE_CAPACITY 8u

typedef enum {
    SLC_RESULT_COMMIT = 0,
    SLC_REJECT_BAD_ARGUMENT,
    SLC_REJECT_SIGNATURE,
    SLC_REJECT_ISSUER,
    SLC_REJECT_NOT_YET_VALID,
    SLC_REJECT_EXPIRED,
    SLC_REJECT_EPOCH,
    SLC_REJECT_TARGET,
    SLC_REJECT_ACTION,
    SLC_REJECT_PAYLOAD,
    SLC_REJECT_NONCE_MISMATCH,
    SLC_REJECT_REPLAY,
    SLC_REJECT_EFFECT_FAILED
} slc_result_t;

typedef struct {
    uint32_t target_id;
    uint32_t action_id;
    uint32_t payload_digest;
    uint64_t nonce;
} slc_request_t;

typedef struct {
    uint32_t target_id;
    uint32_t action_id;
    uint32_t payload_digest;
    uint32_t epoch;
    uint64_t nonce;
    uint64_t not_before;
    uint64_t not_after;
    bool signature_valid;
    bool issuer_authorized;
} slc_authorization_t;

typedef struct {
    uint32_t current_epoch;
    uint64_t replay_nonces[SLC_REPLAY_CACHE_CAPACITY];
    size_t replay_count;
    size_t replay_next;
} slc_gate_t;

typedef bool (*slc_effect_fn)(void *context);

void slc_gate_init(slc_gate_t *gate, uint32_t current_epoch);
slc_result_t slc_gate_execute(
    slc_gate_t *gate,
    const slc_request_t *request,
    const slc_authorization_t *authorization,
    uint64_t now,
    slc_effect_fn effect,
    void *effect_context
);
const char *slc_result_name(slc_result_t result);

#ifdef __cplusplus
}
#endif

#endif
