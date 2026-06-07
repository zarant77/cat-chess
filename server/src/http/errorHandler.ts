import type { FastifyError, FastifyInstance, FastifyReply, FastifyRequest } from "fastify";
import { ZodError } from "zod";
import { ApiError } from "./apiError.js";

export function registerErrorHandler(app: FastifyInstance): void {
  app.setErrorHandler(async (error: FastifyError | Error, _request: FastifyRequest, reply: FastifyReply) => {
    if (error instanceof ApiError) {
      return reply.status(error.statusCode).send({
        error: error.code,
      });
    }

    if (error instanceof ZodError) {
      return reply.status(400).send({
        error: "invalid_request",
        issues: error.issues,
      });
    }

    app.log.error(error);

    return reply.status(500).send({
      error: "internal_server_error",
    });
  });
}
