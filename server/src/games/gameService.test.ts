import { beforeEach, describe, expect, it } from "vitest";
import { db } from "../db/database.js";
import { createGame, createOrTouchDevice, getGame, joinGame, listGames } from "./gameService.js";
import { resetTestDatabase } from "../test/testDatabase.js";

describe("gameService", () => {
  beforeEach(() => {
    resetTestDatabase();
  });

  it("creates a device secret", () => {
    const device = createOrTouchDevice();

    expect(device.deviceSecret).toHaveLength(43);
    expect(device.deviceHash).toHaveLength(64);
  });

  it("touches existing device with the same secret", () => {
    const first = createOrTouchDevice();
    const second = createOrTouchDevice(first.deviceSecret);

    expect(second.deviceSecret).toBe(first.deviceSecret);
    expect(second.deviceHash).toBe(first.deviceHash);
  });

  it("rejects invalid device secret", () => {
    expect(() => {
      createOrTouchDevice("bad-secret");
    }).toThrow("invalid_device_secret");
  });

  it("creates a waiting game with reserved invite code", () => {
    const white = createOrTouchDevice();
    const game = createGame(white.deviceSecret);

    expect(game.id).toBeGreaterThan(0);
    expect(game.status).toBe("waiting_for_black");
    expect(game.yourColor).toBe("white");
    expect(game.sideToMove).toBe("white");
    expect(game.inviteCode).toBeTruthy();
    expect(game.startedAt).toBeNull();
    expect(game.finishedAt).toBeNull();

    const inviteCode = db
      .prepare(
        `
        SELECT status, reserved_game_id
        FROM invite_codes
        WHERE code = ?
      `,
      )
      .get(game.inviteCode) as {
      status: string;
      reserved_game_id: number;
    };

    expect(inviteCode.status).toBe("reserved");
    expect(inviteCode.reserved_game_id).toBe(game.id);
  });

  it("allows another device to join as black", () => {
    const white = createOrTouchDevice();
    const black = createOrTouchDevice();

    const createdGame = createGame(white.deviceSecret);

    expect(createdGame.inviteCode).not.toBeNull();

    const joinedGame = joinGame(createdGame.inviteCode!, black.deviceSecret);

    expect(joinedGame.id).toBe(createdGame.id);
    expect(joinedGame.status).toBe("active");
    expect(joinedGame.yourColor).toBe("black");
    expect(joinedGame.sideToMove).toBe("white");
    expect(joinedGame.startedAt).not.toBeNull();
    expect(joinedGame.finishedAt).toBeNull();
  });

  it("normalizes invite code on join", () => {
    const white = createOrTouchDevice();
    const black = createOrTouchDevice();

    const createdGame = createGame(white.deviceSecret);

    expect(createdGame.inviteCode).not.toBeNull();

    const joinedGame = joinGame(`  ${createdGame.inviteCode!.toLowerCase()}  `, black.deviceSecret);

    expect(joinedGame.id).toBe(createdGame.id);
    expect(joinedGame.status).toBe("active");
    expect(joinedGame.yourColor).toBe("black");
  });

  it("releases invite code after black joins", () => {
    const white = createOrTouchDevice();
    const black = createOrTouchDevice();

    const createdGame = createGame(white.deviceSecret);
    const inviteCode = createdGame.inviteCode;

    expect(inviteCode).not.toBeNull();

    joinGame(inviteCode!, black.deviceSecret);

    const inviteCodeRow = db
      .prepare(
        `
        SELECT status, reserved_game_id, reserved_at
        FROM invite_codes
        WHERE code = ?
      `,
      )
      .get(inviteCode) as {
      status: string;
      reserved_game_id: number | null;
      reserved_at: number | null;
    };

    expect(inviteCodeRow.status).toBe("free");
    expect(inviteCodeRow.reserved_game_id).toBeNull();
    expect(inviteCodeRow.reserved_at).toBeNull();
  });

  it("does not allow joining own game", () => {
    const white = createOrTouchDevice();
    const createdGame = createGame(white.deviceSecret);

    expect(() => {
      joinGame(createdGame.inviteCode!, white.deviceSecret);
    }).toThrow("cannot_join_own_game");
  });

  it("does not allow joining with invalid invite code", () => {
    const black = createOrTouchDevice();

    expect(() => {
      joinGame("BADCAT", black.deviceSecret);
    }).toThrow("invite_code_not_found");
  });

  it("does not allow two active games between the same pair", () => {
    const white = createOrTouchDevice();
    const black = createOrTouchDevice();

    const firstGame = createGame(white.deviceSecret);
    joinGame(firstGame.inviteCode!, black.deviceSecret);

    const secondGame = createGame(white.deviceSecret);

    expect(() => {
      joinGame(secondGame.inviteCode!, black.deviceSecret);
    }).toThrow("game_between_players_already_exists");
  });

  it("lists games for white device", () => {
    const white = createOrTouchDevice();
    const game = createGame(white.deviceSecret);

    const games = listGames(white.deviceSecret);

    expect(games).toHaveLength(1);
    expect(games[0]?.id).toBe(game.id);
    expect(games[0]?.yourColor).toBe("white");
    expect(games[0]?.status).toBe("waiting_for_black");
  });

  it("lists games for black device after join", () => {
    const white = createOrTouchDevice();
    const black = createOrTouchDevice();

    const createdGame = createGame(white.deviceSecret);
    joinGame(createdGame.inviteCode!, black.deviceSecret);

    const blackGames = listGames(black.deviceSecret);

    expect(blackGames).toHaveLength(1);
    expect(blackGames[0]?.id).toBe(createdGame.id);
    expect(blackGames[0]?.yourColor).toBe("black");
    expect(blackGames[0]?.status).toBe("active");
  });

  it("lists multiple games for same device", () => {
    const player = createOrTouchDevice();

    const gameA = createGame(player.deviceSecret);
    const gameB = createGame(player.deviceSecret);

    const games = listGames(player.deviceSecret);

    expect(games).toHaveLength(2);
    expect(games.map((game) => game.id)).toContain(gameA.id);
    expect(games.map((game) => game.id)).toContain(gameB.id);
  });

  it("gets game by id for white participant", () => {
    const white = createOrTouchDevice();
    const game = createGame(white.deviceSecret);

    const loadedGame = getGame(game.id, white.deviceSecret);

    expect(loadedGame.id).toBe(game.id);
    expect(loadedGame.yourColor).toBe("white");
    expect(loadedGame.status).toBe("waiting_for_black");
  });

  it("gets game by id for black participant", () => {
    const white = createOrTouchDevice();
    const black = createOrTouchDevice();

    const createdGame = createGame(white.deviceSecret);
    joinGame(createdGame.inviteCode!, black.deviceSecret);

    const loadedGame = getGame(createdGame.id, black.deviceSecret);

    expect(loadedGame.id).toBe(createdGame.id);
    expect(loadedGame.yourColor).toBe("black");
    expect(loadedGame.status).toBe("active");
  });

  it("does not get game for non-participant", () => {
    const white = createOrTouchDevice();
    const stranger = createOrTouchDevice();

    const game = createGame(white.deviceSecret);

    expect(() => {
      getGame(game.id, stranger.deviceSecret);
    }).toThrow("game_not_found");
  });

  it("does not list games for non-participant", () => {
    const white = createOrTouchDevice();
    const stranger = createOrTouchDevice();

    createGame(white.deviceSecret);

    const games = listGames(stranger.deviceSecret);

    expect(games).toHaveLength(0);
  });
});
