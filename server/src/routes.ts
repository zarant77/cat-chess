import type { FastifyInstance, FastifyRequest } from "fastify";
import { z } from "zod";
import { createGame, createOrTouchDevice, getGame, joinGame, listGameMoves, listGames, makeMove, resignGame } from "./games/gameService.js";
import { checkDatabaseHealth, listDebugInviteCodes } from "./db/repositories/debugRepository.js";
import { unauthorized } from "./http/apiError.js";

const DEVICE_HEADER_NAME = "x-cat-chess-device";

const deviceBodySchema = z.object({
  deviceSecret: z.string().optional(),
});

const joinGameBodySchema = z.object({
  inviteCode: z.string().min(3).max(12),
});

const moveBodySchema = z.object({
  uci: z.string().min(4).max(5),
});

const gameIdParamsSchema = z.object({
  id: z.coerce.number().int().positive(),
});

function readDeviceSecret(request: FastifyRequest): string {
  const value = request.headers[DEVICE_HEADER_NAME];

  if (typeof value !== "string" || value.length < 16) {
    throw unauthorized("missing_device_secret");
  }

  return value;
}

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
    const deviceSecret = readDeviceSecret(request);

    return createGame(deviceSecret);
  });

  app.get("/games", async (request) => {
    const deviceSecret = readDeviceSecret(request);

    return {
      games: listGames(deviceSecret),
    };
  });

  app.get("/games/:id", async (request) => {
    const deviceSecret = readDeviceSecret(request);
    const params = gameIdParamsSchema.parse(request.params);

    return getGame(params.id, deviceSecret);
  });

  app.get("/games/:id/moves", async (request) => {
    const deviceSecret = readDeviceSecret(request);
    const params = gameIdParamsSchema.parse(request.params);

    return {
      moves: listGameMoves(params.id, deviceSecret),
    };
  });

  app.post("/games/join", async (request) => {
    const deviceSecret = readDeviceSecret(request);
    const body = joinGameBodySchema.parse(request.body);

    return joinGame(body.inviteCode, deviceSecret);
  });

  app.post("/games/:id/move", async (request) => {
    const deviceSecret = readDeviceSecret(request);
    const params = gameIdParamsSchema.parse(request.params);
    const body = moveBodySchema.parse(request.body);

    return makeMove(params.id, deviceSecret, body.uci);
  });

  app.post("/games/:id/resign", async (request) => {
    const deviceSecret = readDeviceSecret(request);
    const params = gameIdParamsSchema.parse(request.params);

    return resignGame(params.id, deviceSecret);
  });

  app.get("/debug/invite-codes", async () => {
    return {
      codes: listDebugInviteCodes(),
    };
  });
}
