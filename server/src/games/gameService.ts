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
  finishGame,
  insertWaitingGame,
  runGameTransaction,
  setGameInviteCode,
  updateGameAfterMove,
  type GameResult,
  type GameRow,
  type PlayerColor,
} from "../db/repositories/gameRepository.js";
import { countMovesForGame, insertMove, listMovesForGame, type MoveRow } from "../db/repositories/moveRepository.js";
import { isBasicUciMove, normalizeUciMove } from "../chess/moveFormat.js";
import { badRequest, conflict, forbidden, notFound } from "../http/apiError.js";

const INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

export interface DeviceSession {
  deviceSecret: string;
  deviceHash: string;
}

export interface GameDto {
  id: number;
  inviteCode: string | null;
  status: string;
  result: GameResult | null;
  boardFen: string;
  sideToMove: PlayerColor;
  yourColor: PlayerColor | null;
  createdAt: number;
  updatedAt: number;
  startedAt: number | null;
  finishedAt: number | null;
}

export interface MoveDto {
  id: number;
  gameId: number;
  moveIndex: number;
  color: PlayerColor;
  uci: string;
  fenAfter: string;
  createdAt: number;
}

export function nowSec(): number {
  return Math.floor(Date.now() / 1000);
}

export function createOrTouchDevice(deviceSecret?: string): DeviceSession {
  const secret = deviceSecret ?? createDeviceSecret();

  if (!isValidDeviceSecret(secret)) {
    throw badRequest("invalid_device_secret");
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
      throw notFound("invite_code_not_found");
    }

    if (waitingGame.white_device_hash === device.deviceHash) {
      throw forbidden("cannot_join_own_game");
    }

    const playerPairKey = createPlayerPairKey(waitingGame.white_device_hash, device.deviceHash);

    const existingGame = findUnfinishedGameByPlayerPairKey(playerPairKey);

    if (existingGame) {
      throw conflict("game_between_players_already_exists");
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
    throw notFound("game_not_found");
  }

  return toGameDto(game, getColorForDevice(game, device.deviceHash));
}

export function listGameMoves(gameId: number, deviceSecret: string): MoveDto[] {
  const device = createOrTouchDevice(deviceSecret);
  const game = findGameByIdForDevice(gameId, device.deviceHash);

  if (!game) {
    throw notFound("game_not_found");
  }

  return listMovesForGame(gameId).map(toMoveDto);
}

export function makeMove(gameId: number, deviceSecret: string, rawUci: string): GameDto {
  const device = createOrTouchDevice(deviceSecret);
  const now = nowSec();
  const uci = normalizeUciMove(rawUci);

  if (!isBasicUciMove(uci)) {
    throw badRequest("invalid_move_format");
  }

  return runGameTransaction(() => {
    const game = findGameById(gameId);

    if (!game) {
      throw notFound("game_not_found");
    }

    if (game.status !== "active") {
      throw conflict("game_not_active");
    }

    const color = getColorForDevice(game, device.deviceHash);

    if (!color) {
      throw forbidden("not_your_game");
    }

    if (game.side_to_move !== color) {
      throw forbidden("not_your_turn");
    }

    const moveIndex = countMovesForGame(gameId);
    const nextSideToMove: PlayerColor = color === "white" ? "black" : "white";

    // TODO: replace with real FEN update after chess rules are implemented.
    const fenAfter = game.board_fen;

    insertMove(gameId, moveIndex, color, uci, fenAfter, now);
    updateGameAfterMove(gameId, nextSideToMove, now);

    const updatedGame = findGameById(gameId);

    if (!updatedGame) {
      throw new Error("game_not_found_after_move");
    }

    return toGameDto(updatedGame, color);
  });
}

export function resignGame(gameId: number, deviceSecret: string): GameDto {
  const device = createOrTouchDevice(deviceSecret);
  const now = nowSec();

  return runGameTransaction(() => {
    const game = findGameById(gameId);

    if (!game) {
      throw notFound("game_not_found");
    }

    if (game.status !== "active") {
      throw conflict("game_not_active");
    }

    const color = getColorForDevice(game, device.deviceHash);

    if (!color) {
      throw forbidden("not_your_game");
    }

    const result: GameResult = color === "white" ? "black_won" : "white_won";

    finishGame(gameId, result, now);

    const updatedGame = findGameById(gameId);

    if (!updatedGame) {
      throw new Error("game_not_found_after_resign");
    }

    return toGameDto(updatedGame, color);
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
    result: row.result,
    boardFen: row.board_fen,
    sideToMove: row.side_to_move,
    yourColor,
    createdAt: row.created_at,
    updatedAt: row.updated_at,
    startedAt: row.started_at,
    finishedAt: row.finished_at,
  };
}

function toMoveDto(row: MoveRow): MoveDto {
  return {
    id: row.id,
    gameId: row.game_id,
    moveIndex: row.move_index,
    color: row.color,
    uci: row.uci,
    fenAfter: row.fen_after,
    createdAt: row.created_at,
  };
}
