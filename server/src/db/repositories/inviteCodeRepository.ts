import { db } from "../database.js";

export interface InviteCodeRow {
  code: string;
  status: string;
  reserved_game_id: number | null;
  reserved_at: number | null;
  used_count: number;
}

export function insertInviteCodeIfMissing(code: string): void {
  db.prepare(
    `
    INSERT OR IGNORE INTO invite_codes (
      code,
      status,
      reserved_game_id,
      reserved_at,
      used_count
    )
    VALUES (?, 'free', NULL, NULL, 0)
  `,
  ).run(code);
}

export function findRandomFreeInviteCode(): InviteCodeRow | undefined {
  return db
    .prepare(
      `
    SELECT code, status, reserved_game_id, reserved_at, used_count
    FROM invite_codes
    WHERE status = 'free'
    ORDER BY RANDOM()
    LIMIT 1
  `,
    )
    .get() as InviteCodeRow | undefined;
}

export function reserveInviteCode(code: string, gameId: number, now: number): boolean {
  const result = db
    .prepare(
      `
    UPDATE invite_codes
    SET status = 'reserved',
        reserved_game_id = ?,
        reserved_at = ?,
        used_count = used_count + 1
    WHERE code = ?
      AND status = 'free'
  `,
    )
    .run(gameId, now, code);

  return result.changes === 1;
}

export function releaseInviteCodeByGameId(gameId: number): void {
  db.prepare(
    `
    UPDATE invite_codes
    SET status = 'free',
        reserved_game_id = NULL,
        reserved_at = NULL
    WHERE reserved_game_id = ?
  `,
  ).run(gameId);
}
