import { db } from "../database.js";

export type GameStatus = "waiting_for_black" | "active" | "finished";
export type GameResult = "white_won" | "black_won" | "draw";
export type PlayerColor = "white" | "black";

export interface GameRow {
  id: number;
  invite_code: string | null;
  status: GameStatus;
  result: GameResult | null;
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
      result,
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
      NULL,
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

export function findWaitingGameByInviteCode(inviteCode: string): GameRow | undefined {
  return db
    .prepare(
      `
    SELECT *
    FROM games
    WHERE invite_code = ?
      AND status = 'waiting_for_black'
    LIMIT 1
  `,
    )
    .get(inviteCode) as GameRow | undefined;
}

export function findUnfinishedGameByPlayerPairKey(playerPairKey: string): GameRow | undefined {
  return db
    .prepare(
      `
    SELECT *
    FROM games
    WHERE player_pair_key = ?
      AND finished_at IS NULL
    LIMIT 1
  `,
    )
    .get(playerPairKey) as GameRow | undefined;
}

export function activateGame(gameId: number, blackDeviceHash: string, playerPairKey: string, now: number): void {
  db.prepare(
    `
    UPDATE games
    SET status = 'active',
        black_device_hash = ?,
        player_pair_key = ?,
        started_at = ?,
        updated_at = ?
    WHERE id = ?
      AND status = 'waiting_for_black'
  `,
  ).run(blackDeviceHash, playerPairKey, now, now, gameId);
}

export function findGamesByDeviceHash(deviceHash: string): GameRow[] {
  return db
    .prepare(
      `
    SELECT *
    FROM games
    WHERE white_device_hash = ?
       OR black_device_hash = ?
    ORDER BY updated_at DESC
  `,
    )
    .all(deviceHash, deviceHash) as GameRow[];
}

export function findGameByIdForDevice(gameId: number, deviceHash: string): GameRow | undefined {
  return db
    .prepare(
      `
    SELECT *
    FROM games
    WHERE id = ?
      AND (
        white_device_hash = ?
        OR black_device_hash = ?
      )
    LIMIT 1
  `,
    )
    .get(gameId, deviceHash, deviceHash) as GameRow | undefined;
}

export function updateGameAfterMove(
  gameId: number,
  boardFen: string,
  nextSideToMove: PlayerColor,
  status: GameStatus,
  result: GameResult | null,
  finishedAt: number | null,
  now: number,
): void {
  db.prepare(
    `
    UPDATE games
    SET board_fen = ?,
        side_to_move = ?,
        status = ?,
        result = ?,
        updated_at = ?,
        finished_at = ?
    WHERE id = ?
  `,
  ).run(boardFen, nextSideToMove, status, result, now, finishedAt, gameId);
}

export function finishGame(gameId: number, result: GameResult, now: number): void {
  db.prepare(
    `
    UPDATE games
    SET status = 'finished',
        result = ?,
        updated_at = ?,
        finished_at = ?
    WHERE id = ?
      AND status = 'active'
  `,
  ).run(result, now, now, gameId);
}

export function runGameTransaction<T>(callback: () => T): T {
  return db.transaction(callback)();
}
