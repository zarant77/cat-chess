import { db } from "../database.js";
import type { PlayerColor } from "./gameRepository.js";

export interface MoveRow {
  id: number;
  game_id: number;
  move_index: number;
  color: PlayerColor;
  uci: string;
  fen_after: string;
  created_at: number;
}

export function countMovesForGame(gameId: number): number {
  const row = db
    .prepare(
      `
      SELECT COUNT(*) AS count
      FROM moves
      WHERE game_id = ?
    `,
    )
    .get(gameId) as { count: number };

  return row.count;
}

export function insertMove(gameId: number, moveIndex: number, color: PlayerColor, uci: string, fenAfter: string, now: number): number {
  const result = db
    .prepare(
      `
      INSERT INTO moves (
        game_id,
        move_index,
        color,
        uci,
        fen_after,
        created_at
      )
      VALUES (?, ?, ?, ?, ?, ?)
    `,
    )
    .run(gameId, moveIndex, color, uci, fenAfter, now);

  return Number(result.lastInsertRowid);
}

export function listMovesForGame(gameId: number): MoveRow[] {
  return db
    .prepare(
      `
      SELECT *
      FROM moves
      WHERE game_id = ?
      ORDER BY move_index ASC
    `,
    )
    .all(gameId) as MoveRow[];
}

export function findLatestMoveForGame(gameId: number): MoveRow | undefined {
  return db
    .prepare(
      `
      SELECT *
      FROM moves
      WHERE game_id = ?
      ORDER BY move_index DESC
      LIMIT 1
    `,
    )
    .get(gameId) as MoveRow | undefined;
}
