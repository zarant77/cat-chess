import { db } from "../db/database.js";
import { seedInviteCodes } from "../games/inviteCodes.js";

export function resetTestDatabase(): void {
  db.prepare("DELETE FROM moves").run();
  db.prepare("DELETE FROM games").run();
  db.prepare("DELETE FROM invite_codes").run();
  db.prepare("DELETE FROM devices").run();

  seedInviteCodes();
}
