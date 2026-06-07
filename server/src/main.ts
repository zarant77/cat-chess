import cors from "@fastify/cors";
import Fastify from "fastify";
import { seedInviteCodes } from "./games/inviteCodes.js";
import { registerErrorHandler } from "./http/errorHandler.js";
import { registerRoutes } from "./routes.js";

const app = Fastify({
  logger: true,
});

registerErrorHandler(app);

await app.register(cors, {
  origin: true,
});

seedInviteCodes();

await registerRoutes(app);

const port = Number(process.env.PORT || 5400);
const host = process.env.HOST || "0.0.0.0";

await app.listen({ port, host });
