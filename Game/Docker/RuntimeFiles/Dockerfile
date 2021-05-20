FROM alpine:3
COPY --chown=worker:worker ./schema.sb /improbable/workers/sample-game/schema/schema.sb
COPY --chown=worker:worker ./default.snapshot /improbable/workers/sample-game/schema/default.snapshot
RUN chmod 744 /improbable/workers/sample-game/schema/schema.sb
RUN chmod 744 /improbable/workers/sample-game/schema/default.snapshot