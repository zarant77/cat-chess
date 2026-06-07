import { db } from "../database.js";

export interface DebugInviteCodeRow {
  code: string;
  status: string;
  reserved_game_id: number | null;
  used_count: number;
}

export function listDebugInviteCodes(): DebugInviteCodeRow[] {
  return db
    .prepare(
      `
    SELECT code, status, reserved_game_id, used_count
    FROM invite_codes
    ORDER BY code
  `,
    )
    .all() as DebugInviteCodeRow[];
}

export function checkDatabaseHealth(): boolean {
  const row = db.prepare("SELECT 1 AS ok").get() as { ok: number };
  return row.ok === 1;
}
