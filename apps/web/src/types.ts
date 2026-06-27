export type PlayerColor = "white" | "black";
export type GameStatus = "waiting_for_black" | "active" | "finished";
export type GameResult = "white_won" | "black_won" | "draw" | null;

export interface GameDto {
  id: number;
  inviteCode: string | null;
  status: GameStatus;
  result: GameResult;
  boardFen: string;
  sideToMove: PlayerColor;
  yourColor: PlayerColor | null;
  lastMove: string | null;
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

export interface ApiErrorResponse {
  error: string;
  issues?: unknown[];
}

export interface ListGamesResponse {
  games: GameDto[];
}

export interface ListMovesResponse {
  moves: MoveDto[];
}
