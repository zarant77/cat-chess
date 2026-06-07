import { createDeviceSecret, hashDeviceSecret, isValidDeviceSecret } from "../security/device.js";
import { upsertDevice } from "../db/repositories/deviceRepository.js";
import { reserveInviteCode } from "./inviteCodes.js";
import {
  findGameById,
  insertWaitingGame,
  runGameTransaction,
  setGameInviteCode,
  type GameRow,
  type PlayerColor,
} from "../db/repositories/gameRepository.js";

const INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

export interface DeviceSession {
  deviceSecret: string;
  deviceHash: string;
}

export interface GameDto {
  id: number;
  inviteCode: string | null;
  status: string;
  boardFen: string;
  sideToMove: PlayerColor;
  yourColor: PlayerColor | null;
  createdAt: number;
  updatedAt: number;
  startedAt: number | null;
  finishedAt: number | null;
}

export function nowSec(): number {
  return Math.floor(Date.now() / 1000);
}

export function createOrTouchDevice(deviceSecret?: string): DeviceSession {
  const secret = deviceSecret ?? createDeviceSecret();

  if (!isValidDeviceSecret(secret)) {
    throw new Error("invalid_device_secret");
  }

  const deviceHash = hashDeviceSecret(secret);
  const now = nowSec();

  upsertDevice(deviceHash, now);

  return {
    deviceSecret: secret,
    deviceHash,
  };
}

export function createGame(deviceSecret: string): GameDto {
  const device = createOrTouchDevice(deviceSecret);
  const now = nowSec();

  return runGameTransaction(() => {
    const gameId = insertWaitingGame(INITIAL_FEN, device.deviceHash, now);
    const inviteCode = reserveInviteCode(gameId, now);

    setGameInviteCode(gameId, inviteCode, now);

    const game = findGameById(gameId);

    if (!game) {
      throw new Error("game_not_found_after_create");
    }

    return toGameDto(game, "white");
  });
}

function toGameDto(row: GameRow, yourColor: PlayerColor | null): GameDto {
  return {
    id: row.id,
    inviteCode: row.invite_code,
    status: row.status,
    boardFen: row.board_fen,
    sideToMove: row.side_to_move,
    yourColor,
    createdAt: row.created_at,
    updatedAt: row.updated_at,
    startedAt: row.started_at,
    finishedAt: row.finished_at,
  };
}
