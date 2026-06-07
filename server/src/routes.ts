import type { FastifyInstance } from "fastify";
import { z } from "zod";
import { createGame, createOrTouchDevice } from "./games/gameService.js";
import { checkDatabaseHealth, listDebugInviteCodes } from "./db/repositories/debugRepository.js";

const deviceBodySchema = z.object({
  deviceSecret: z.string().optional(),
});

const authBodySchema = z.object({
  deviceSecret: z.string().min(16),
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

  app.get("/debug/invite-codes", async () => {
    return {
      codes: listDebugInviteCodes(),
    };
  });
}
