import type { FastifyInstance } from "fastify";
import { z } from "zod";
import { createGame, createOrTouchDevice, getGame, joinGame, listGames } from "./games/gameService.js";
import { checkDatabaseHealth, listDebugInviteCodes } from "./db/repositories/debugRepository.js";

const deviceBodySchema = z.object({
  deviceSecret: z.string().optional(),
});

const authBodySchema = z.object({
  deviceSecret: z.string().min(16),
});

const authQuerySchema = z.object({
  deviceSecret: z.string().min(16),
});

const joinGameBodySchema = z.object({
  deviceSecret: z.string().min(16),
  inviteCode: z.string().min(3).max(12),
});

const gameIdParamsSchema = z.object({
  id: z.coerce.number().int().positive(),
});

export async function registerRoutes(app: FastifyInstance): Promise<void> {
  app.get("/health", async () => {
    return {
      ok: checkDatabaseHealth(),
      service: "cat-chess-server",
    };
  });

  app.post("/device", async (request) => {
    const body = deviceBodySchema.parse(request.body);
    const device = createOrTouchDevice(body.deviceSecret);

    return {
      deviceSecret: device.deviceSecret,
    };
  });

  app.post("/games", async (request) => {
    const body = authBodySchema.parse(request.body);

    return createGame(body.deviceSecret);
  });

  app.get("/games", async (request) => {
    const query = authQuerySchema.parse(request.query);

    return {
      games: listGames(query.deviceSecret),
    };
  });

  app.get("/games/:id", async (request) => {
    const params = gameIdParamsSchema.parse(request.params);
    const query = authQuerySchema.parse(request.query);

    return getGame(params.id, query.deviceSecret);
  });

  app.post("/games/join", async (request) => {
    const body = joinGameBodySchema.parse(request.body);

    return joinGame(body.inviteCode, body.deviceSecret);
  });

  app.get("/debug/invite-codes", async () => {
    return {
      codes: listDebugInviteCodes(),
    };
  });
}
