import { db } from "../database.js";

export type GameStatus = "waiting_for_black" | "active" | "finished";
export type PlayerColor = "white" | "black";

export interface GameRow {
  id: number;
  invite_code: string | null;
  status: GameStatus;
  board_fen: string;
  side_to_move: PlayerColor;
  white_device_hash: string;
  black_device_hash: string | null;
  player_pair_key: string | null;
  created_at: number;
  updated_at: number;
  started_at: number | null;
  finished_at: number | null;
}

export function insertWaitingGame(boardFen: string, whiteDeviceHash: string, now: number): number {
  const result = db
    .prepare(
      `
    INSERT INTO games (
      invite_code,
      status,
      board_fen,
      side_to_move,
      white_device_hash,
      black_device_hash,
      player_pair_key,
      created_at,
      updated_at,
      started_at,
      finished_at
    )
    VALUES (
      NULL,
      'waiting_for_black',
      ?,
      'white',
      ?,
      NULL,
      NULL,
      ?,
      ?,
      NULL,
      NULL
    )
  `,
    )
    .run(boardFen, whiteDeviceHash, now, now);

  return Number(result.lastInsertRowid);
}

export function setGameInviteCode(gameId: number, inviteCode: string, now: number): void {
  db.prepare(
    `
    UPDATE games
    SET invite_code = ?,
        updated_at = ?
    WHERE id = ?
  `,
  ).run(inviteCode, now, gameId);
}

export function findGameById(gameId: number): GameRow | undefined {
  return db
    .prepare(
      `
    SELECT *
    FROM games
    WHERE id = ?
  `,
    )
    .get(gameId) as GameRow | undefined;
}

export function runGameTransaction<T>(callback: () => T): T {
  return db.transaction(callback)();
}
