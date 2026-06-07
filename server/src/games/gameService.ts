import { createDeviceSecret, createPlayerPairKey, hashDeviceSecret, isValidDeviceSecret } from "../security/device.js";
import { upsertDevice } from "../db/repositories/deviceRepository.js";
import { releaseInviteCodeForGame, reserveInviteCode } from "./inviteCodes.js";
import {
  activateGame,
  findGameById,
  findGameByIdForDevice,
  findGamesByDeviceHash,
  findUnfinishedGameByPlayerPairKey,
  findWaitingGameByInviteCode,
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

export function joinGame(inviteCode: string, deviceSecret: string): GameDto {
  const device = createOrTouchDevice(deviceSecret);
  const now = nowSec();
  const normalizedInviteCode = inviteCode.trim().toUpperCase();

  return runGameTransaction(() => {
    const waitingGame = findWaitingGameByInviteCode(normalizedInviteCode);

    if (!waitingGame) {
      throw new Error("invite_code_not_found");
    }

    if (waitingGame.white_device_hash === device.deviceHash) {
      throw new Error("cannot_join_own_game");
    }

    const playerPairKey = createPlayerPairKey(waitingGame.white_device_hash, device.deviceHash);

    const existingGame = findUnfinishedGameByPlayerPairKey(playerPairKey);

    if (existingGame) {
      throw new Error("game_between_players_already_exists");
    }

    activateGame(waitingGame.id, device.deviceHash, playerPairKey, now);
    releaseInviteCodeForGame(waitingGame.id);

    const game = findGameById(waitingGame.id);

    if (!game) {
      throw new Error("game_not_found_after_join");
    }

    return toGameDto(game, "black");
  });
}

function getColorForDevice(row: GameRow, deviceHash: string): PlayerColor | null {
  if (row.white_device_hash === deviceHash) {
    return "white";
  }

  if (row.black_device_hash === deviceHash) {
    return "black";
  }

  return null;
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

export function listGames(deviceSecret: string): GameDto[] {
  const device = createOrTouchDevice(deviceSecret);
  const games = findGamesByDeviceHash(device.deviceHash);

  return games.map((game) => {
    return toGameDto(game, getColorForDevice(game, device.deviceHash));
  });
}

export function getGame(gameId: number, deviceSecret: string): GameDto {
  const device = createOrTouchDevice(deviceSecret);
  const game = findGameByIdForDevice(gameId, device.deviceHash);

  if (!game) {
    throw new Error("game_not_found");
  }

  return toGameDto(game, getColorForDevice(game, device.deviceHash));
}
