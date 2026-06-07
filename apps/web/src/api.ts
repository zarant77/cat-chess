import type { GameDto, ListGamesResponse, ListMovesResponse } from "./types";

const API_BASE_URL = import.meta.env.VITE_CAT_CHESS_API_URL ?? "http://localhost:5400";
const DEVICE_HEADER_NAME = "X-Cat-Chess-Device";

interface DeviceResponse {
  deviceSecret: string;
}

async function requestJson<T>(path: string, options: RequestInit = {}, deviceSecret?: string): Promise<T> {
  const headers = new Headers(options.headers);

  if (!headers.has("content-type") && options.body) {
    headers.set("content-type", "application/json");
  }

  if (deviceSecret) {
    headers.set(DEVICE_HEADER_NAME, deviceSecret);
  }

  const response = await fetch(`${API_BASE_URL}${path}`, {
    ...options,
    headers,
  });

  const data = (await response.json().catch(() => null)) as unknown;

  if (!response.ok) {
    const errorCode =
      typeof data === "object" && data !== null && "error" in data && typeof data.error === "string"
        ? data.error
        : `http_${response.status}`;

    throw new Error(errorCode);
  }

  return data as T;
}

export async function createDevice(deviceSecret?: string): Promise<DeviceResponse> {
  return requestJson<DeviceResponse>("/device", {
    method: "POST",
    body: JSON.stringify(deviceSecret ? { deviceSecret } : {}),
  });
}

export async function createGame(deviceSecret: string): Promise<GameDto> {
  return requestJson<GameDto>(
    "/games",
    {
      method: "POST",
      body: JSON.stringify({}),
    },
    deviceSecret,
  );
}

export async function joinGame(deviceSecret: string, inviteCode: string): Promise<GameDto> {
  return requestJson<GameDto>(
    "/games/join",
    {
      method: "POST",
      body: JSON.stringify({ inviteCode }),
    },
    deviceSecret,
  );
}

export async function listGames(deviceSecret: string): Promise<ListGamesResponse> {
  return requestJson<ListGamesResponse>("/games", {}, deviceSecret);
}

export async function getGame(deviceSecret: string, gameId: number): Promise<GameDto> {
  return requestJson<GameDto>(`/games/${gameId}`, {}, deviceSecret);
}

export async function listMoves(deviceSecret: string, gameId: number): Promise<ListMovesResponse> {
  return requestJson<ListMovesResponse>(`/games/${gameId}/moves`, {}, deviceSecret);
}

export async function makeMove(deviceSecret: string, gameId: number, uci: string): Promise<GameDto> {
  return requestJson<GameDto>(
    `/games/${gameId}/move`,
    {
      method: "POST",
      body: JSON.stringify({ uci }),
    },
    deviceSecret,
  );
}

export async function resignGame(deviceSecret: string, gameId: number): Promise<GameDto> {
  return requestJson<GameDto>(
    `/games/${gameId}/resign`,
    {
      method: "POST",
      body: JSON.stringify({}),
    },
    deviceSecret,
  );
}
