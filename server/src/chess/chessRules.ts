import type { PlayerColor } from "../db/repositories/gameRepository.js";

type PieceType = "p" | "n" | "b" | "r" | "q" | "k";

interface Piece {
  type: PieceType;
  color: PlayerColor;
}

interface CastlingRights {
  whiteKingSide: boolean;
  whiteQueenSide: boolean;
  blackKingSide: boolean;
  blackQueenSide: boolean;
}

interface BoardState {
  squares: Array<Piece | null>;
  sideToMove: PlayerColor;
  castling: CastlingRights;
  enPassant: number | null;
  halfmoveClock: number;
  fullmoveNumber: number;
}

interface ParsedMove {
  from: number;
  to: number;
  promotion: PieceType | null;
}

export type ChessMoveResultStatus = "active" | "checkmate" | "stalemate";

export interface ChessMoveResult {
  fen: string;
  sideToMove: PlayerColor;
  status: ChessMoveResultStatus;
}

const promotionPieces = new Set<PieceType>(["q", "r", "b", "n"]);

export function applyLegalUciMove(fen: string, uci: string): ChessMoveResult | null {
  const state = parseFen(fen);
  const move = parseUciMove(uci);

  if (!state || !move || !isLegalMove(state, move)) {
    return null;
  }

  const nextState = applyMoveUnchecked(state, move);
  const legalReplies = generateLegalMoves(nextState);
  let status: ChessMoveResultStatus = "active";

  if (legalReplies.length === 0) {
    status = isInCheck(nextState, nextState.sideToMove) ? "checkmate" : "stalemate";
  }

  return {
    fen: stateToFen(nextState),
    sideToMove: nextState.sideToMove,
    status,
  };
}

function parseFen(fen: string): BoardState | null {
  const parts = fen.trim().split(/\s+/);

  if (parts.length < 4) {
    return null;
  }

  const squares: Array<Piece | null> = [];
  const ranks = parts[0]?.split("/") ?? [];

  if (ranks.length !== 8) {
    return null;
  }

  for (const rank of ranks) {
    for (const char of rank) {
      const empty = Number(char);

      if (Number.isInteger(empty) && empty > 0) {
        for (let index = 0; index < empty; index += 1) {
          squares.push(null);
        }
        continue;
      }

      const lower = char.toLowerCase() as PieceType;
      if (!["p", "n", "b", "r", "q", "k"].includes(lower)) {
        return null;
      }

      squares.push({
        type: lower,
        color: char === lower ? "black" : "white",
      });
    }
  }

  if (squares.length !== 64 || (parts[1] !== "w" && parts[1] !== "b")) {
    return null;
  }

  return {
    squares,
    sideToMove: parts[1] === "w" ? "white" : "black",
    castling: parseCastling(parts[2] ?? "-"),
    enPassant: parts[3] === "-" ? null : squareToIndex(parts[3] ?? ""),
    halfmoveClock: parseFenNumber(parts[4], 0),
    fullmoveNumber: parseFenNumber(parts[5], 1),
  };
}

function parseCastling(text: string): CastlingRights {
  return {
    whiteKingSide: text.includes("K"),
    whiteQueenSide: text.includes("Q"),
    blackKingSide: text.includes("k"),
    blackQueenSide: text.includes("q"),
  };
}

function parseFenNumber(value: string | undefined, fallback: number): number {
  if (!value || !/^\d+$/.test(value)) {
    return fallback;
  }

  return Number(value);
}

function parseUciMove(uci: string): ParsedMove | null {
  const from = squareToIndex(uci.slice(0, 2));
  const to = squareToIndex(uci.slice(2, 4));
  const promotion = uci.length === 5 ? (uci[4] as PieceType | undefined) : null;

  if (from === null || to === null) {
    return null;
  }
  if (promotion && !promotionPieces.has(promotion)) {
    return null;
  }

  return {
    from,
    to,
    promotion: promotion ?? null,
  };
}

function squareToIndex(square: string): number | null {
  if (!/^[a-h][1-8]$/.test(square)) {
    return null;
  }

  const file = square.charCodeAt(0) - "a".charCodeAt(0);
  const rank = Number(square[1]);
  return indexFor(file, rank);
}

function indexToSquare(index: number): string {
  const file = fileOf(index);
  const rank = rankOf(index);
  return `${String.fromCharCode("a".charCodeAt(0) + file)}${rank}`;
}

function indexFor(file: number, rank: number): number | null {
  if (file < 0 || file > 7 || rank < 1 || rank > 8) {
    return null;
  }

  return (8 - rank) * 8 + file;
}

function fileOf(index: number): number {
  return index % 8;
}

function rankOf(index: number): number {
  return 8 - Math.floor(index / 8);
}

function opposite(color: PlayerColor): PlayerColor {
  return color === "white" ? "black" : "white";
}

function isLegalMove(state: BoardState, move: ParsedMove): boolean {
  return generateLegalMoves(state).some((candidate) => {
    return candidate.from === move.from && candidate.to === move.to && candidate.promotion === normalizedPromotion(state, move);
  });
}

function normalizedPromotion(state: BoardState, move: ParsedMove): PieceType | null {
  const piece = state.squares[move.from];

  if (!piece || piece.type !== "p" || (rankOf(move.to) !== 1 && rankOf(move.to) !== 8)) {
    return move.promotion;
  }

  return move.promotion ?? "q";
}

function generateLegalMoves(state: BoardState): ParsedMove[] {
  const pseudoMoves = generatePseudoMoves(state);
  const legalMoves: ParsedMove[] = [];

  for (const move of pseudoMoves) {
    const piece = state.squares[move.from];

    if (!piece) {
      continue;
    }

    const nextState = applyMoveUnchecked(state, move);
    if (!isInCheck(nextState, piece.color)) {
      legalMoves.push(move);
    }
  }

  return legalMoves;
}

function generatePseudoMoves(state: BoardState): ParsedMove[] {
  const moves: ParsedMove[] = [];

  for (let index = 0; index < 64; index += 1) {
    const piece = state.squares[index];

    if (!piece || piece.color !== state.sideToMove) {
      continue;
    }

    if (piece.type === "p") {
      addPawnMoves(state, index, moves);
    } else if (piece.type === "n") {
      addStepMoves(state, index, moves, [
        [1, 2],
        [2, 1],
        [2, -1],
        [1, -2],
        [-1, -2],
        [-2, -1],
        [-2, 1],
        [-1, 2],
      ]);
    } else if (piece.type === "b") {
      addSlideMoves(state, index, moves, [
        [1, 1],
        [1, -1],
        [-1, -1],
        [-1, 1],
      ]);
    } else if (piece.type === "r") {
      addSlideMoves(state, index, moves, [
        [1, 0],
        [0, -1],
        [-1, 0],
        [0, 1],
      ]);
    } else if (piece.type === "q") {
      addSlideMoves(state, index, moves, [
        [1, 1],
        [1, -1],
        [-1, -1],
        [-1, 1],
        [1, 0],
        [0, -1],
        [-1, 0],
        [0, 1],
      ]);
    } else if (piece.type === "k") {
      addStepMoves(state, index, moves, [
        [1, 1],
        [1, 0],
        [1, -1],
        [0, -1],
        [-1, -1],
        [-1, 0],
        [-1, 1],
        [0, 1],
      ]);
      addCastleMoves(state, index, moves);
    }
  }

  return moves;
}

function addPawnMoves(state: BoardState, from: number, moves: ParsedMove[]): void {
  const piece = state.squares[from];

  if (!piece) {
    return;
  }

  const direction = piece.color === "white" ? 1 : -1;
  const startRank = piece.color === "white" ? 2 : 7;
  const one = indexFor(fileOf(from), rankOf(from) + direction);

  if (one !== null && !state.squares[one]) {
    addPawnMove(from, one, piece.color, moves);

    const two = indexFor(fileOf(from), rankOf(from) + direction * 2);
    if (rankOf(from) === startRank && two !== null && !state.squares[two]) {
      moves.push({ from, to: two, promotion: null });
    }
  }

  for (const fileDelta of [-1, 1]) {
    const target = indexFor(fileOf(from) + fileDelta, rankOf(from) + direction);

    if (target === null) {
      continue;
    }

    const targetPiece = state.squares[target];
    if ((targetPiece && targetPiece.color !== piece.color && targetPiece.type !== "k") || target === state.enPassant) {
      addPawnMove(from, target, piece.color, moves);
    }
  }
}

function addPawnMove(from: number, to: number, color: PlayerColor, moves: ParsedMove[]): void {
  if ((color === "white" && rankOf(to) === 8) || (color === "black" && rankOf(to) === 1)) {
    for (const promotion of promotionPieces) {
      moves.push({ from, to, promotion });
    }
    return;
  }

  moves.push({ from, to, promotion: null });
}

function addStepMoves(state: BoardState, from: number, moves: ParsedMove[], offsets: number[][]): void {
  const piece = state.squares[from];

  if (!piece) {
    return;
  }

  for (const [fileStep, rankStep] of offsets) {
    const target = indexFor(fileOf(from) + fileStep, rankOf(from) + rankStep);

    if (target === null) {
      continue;
    }

    const targetPiece = state.squares[target];
    if (!targetPiece || (targetPiece.color !== piece.color && targetPiece.type !== "k")) {
      moves.push({ from, to: target, promotion: null });
    }
  }
}

function addSlideMoves(state: BoardState, from: number, moves: ParsedMove[], directions: number[][]): void {
  const piece = state.squares[from];

  if (!piece) {
    return;
  }

  for (const [fileStep, rankStep] of directions) {
    let file = fileOf(from) + fileStep;
    let rank = rankOf(from) + rankStep;

    while (true) {
      const target = indexFor(file, rank);

      if (target === null) {
        break;
      }

      const targetPiece = state.squares[target];
      if (!targetPiece) {
        moves.push({ from, to: target, promotion: null });
      } else {
        if (targetPiece.color !== piece.color && targetPiece.type !== "k") {
          moves.push({ from, to: target, promotion: null });
        }
        break;
      }

      file += fileStep;
      rank += rankStep;
    }
  }
}

function addCastleMoves(state: BoardState, from: number, moves: ParsedMove[]): void {
  const piece = state.squares[from];

  if (!piece || isInCheck(state, piece.color)) {
    return;
  }

  if (piece.color === "white" && from === indexFor(4, 1)) {
    maybeAddCastle(state, moves, "white", true);
    maybeAddCastle(state, moves, "white", false);
  } else if (piece.color === "black" && from === indexFor(4, 8)) {
    maybeAddCastle(state, moves, "black", true);
    maybeAddCastle(state, moves, "black", false);
  }
}

function maybeAddCastle(state: BoardState, moves: ParsedMove[], color: PlayerColor, kingSide: boolean): void {
  const rank = color === "white" ? 1 : 8;
  const enemy = opposite(color);
  const kingFrom = indexFor(4, rank);
  const kingTo = indexFor(kingSide ? 6 : 2, rank);
  const rookSquare = indexFor(kingSide ? 7 : 0, rank);
  const clearFiles = kingSide ? [5, 6] : [1, 2, 3];
  const safeFiles = kingSide ? [5, 6] : [3, 2];
  const hasRight =
    color === "white"
      ? kingSide
        ? state.castling.whiteKingSide
        : state.castling.whiteQueenSide
      : kingSide
        ? state.castling.blackKingSide
        : state.castling.blackQueenSide;

  if (!hasRight || kingFrom === null || kingTo === null || rookSquare === null) {
    return;
  }

  const rook = state.squares[rookSquare];
  if (!rook || rook.type !== "r" || rook.color !== color) {
    return;
  }

  for (const file of clearFiles) {
    const square = indexFor(file, rank);
    if (square === null || state.squares[square]) {
      return;
    }
  }

  for (const file of safeFiles) {
    const square = indexFor(file, rank);
    if (square === null || isSquareAttacked(state, square, enemy)) {
      return;
    }
  }

  moves.push({ from: kingFrom, to: kingTo, promotion: null });
}

function applyMoveUnchecked(state: BoardState, move: ParsedMove): BoardState {
  const next: BoardState = {
    squares: state.squares.map((piece) => (piece ? { ...piece } : null)),
    sideToMove: opposite(state.sideToMove),
    castling: { ...state.castling },
    enPassant: null,
    halfmoveClock: state.halfmoveClock + 1,
    fullmoveNumber: state.sideToMove === "black" ? state.fullmoveNumber + 1 : state.fullmoveNumber,
  };
  const moving = next.squares[move.from];
  const captured = next.squares[move.to];

  if (!moving) {
    return next;
  }

  const isEnPassantCapture = moving.type === "p" && move.to === state.enPassant && !captured && fileOf(move.from) !== fileOf(move.to);

  next.squares[move.to] = moving;
  next.squares[move.from] = null;

  if (isEnPassantCapture) {
    const capturedPawn = indexFor(fileOf(move.to), rankOf(move.from));
    if (capturedPawn !== null) {
      next.squares[capturedPawn] = null;
    }
  }

  if (moving.type === "k" && Math.abs(fileOf(move.to) - fileOf(move.from)) === 2) {
    moveCastleRook(next, moving.color, fileOf(move.to) === 6);
  }

  if (moving.type === "p" && (rankOf(move.to) === 1 || rankOf(move.to) === 8)) {
    next.squares[move.to] = {
      type: normalizedPromotion(state, move) ?? "q",
      color: moving.color,
    };
  }

  if (moving.type === "p" && Math.abs(rankOf(move.to) - rankOf(move.from)) === 2) {
    next.enPassant = indexFor(fileOf(move.from), (rankOf(move.from) + rankOf(move.to)) / 2);
  }

  if (moving.type === "p" || captured || isEnPassantCapture) {
    next.halfmoveClock = 0;
  }

  clearCastlingForMove(next, move.from);
  clearCastlingForMove(next, move.to);
  if (moving.type === "k") {
    if (moving.color === "white") {
      next.castling.whiteKingSide = false;
      next.castling.whiteQueenSide = false;
    } else {
      next.castling.blackKingSide = false;
      next.castling.blackQueenSide = false;
    }
  }

  return next;
}

function moveCastleRook(state: BoardState, color: PlayerColor, kingSide: boolean): void {
  const rank = color === "white" ? 1 : 8;
  const rookFrom = indexFor(kingSide ? 7 : 0, rank);
  const rookTo = indexFor(kingSide ? 5 : 3, rank);

  if (rookFrom === null || rookTo === null) {
    return;
  }

  state.squares[rookTo] = state.squares[rookFrom];
  state.squares[rookFrom] = null;
}

function clearCastlingForMove(state: BoardState, square: number): void {
  if (square === indexFor(0, 1)) {
    state.castling.whiteQueenSide = false;
  } else if (square === indexFor(7, 1)) {
    state.castling.whiteKingSide = false;
  } else if (square === indexFor(0, 8)) {
    state.castling.blackQueenSide = false;
  } else if (square === indexFor(7, 8)) {
    state.castling.blackKingSide = false;
  }
}

function isInCheck(state: BoardState, color: PlayerColor): boolean {
  const kingSquare = state.squares.findIndex((piece) => piece?.type === "k" && piece.color === color);

  if (kingSquare < 0) {
    return false;
  }

  return isSquareAttacked(state, kingSquare, opposite(color));
}

function isSquareAttacked(state: BoardState, square: number, byColor: PlayerColor): boolean {
  for (let from = 0; from < 64; from += 1) {
    const piece = state.squares[from];

    if (!piece || piece.color !== byColor) {
      continue;
    }

    if (canPieceAttackSquare(state, from, square)) {
      return true;
    }
  }

  return false;
}

function canPieceAttackSquare(state: BoardState, from: number, square: number): boolean {
  const piece = state.squares[from];
  const fileDelta = fileOf(square) - fileOf(from);
  const rankDelta = rankOf(square) - rankOf(from);

  if (!piece) {
    return false;
  }

  if (piece.type === "p") {
    return rankDelta === (piece.color === "white" ? 1 : -1) && Math.abs(fileDelta) === 1;
  }
  if (piece.type === "n") {
    return (Math.abs(fileDelta) === 1 && Math.abs(rankDelta) === 2) || (Math.abs(fileDelta) === 2 && Math.abs(rankDelta) === 1);
  }
  if (piece.type === "b") {
    return Math.abs(fileDelta) === Math.abs(rankDelta) && fileDelta !== 0 && pathClear(state, from, square, Math.sign(fileDelta), Math.sign(rankDelta));
  }
  if (piece.type === "r") {
    return (
      (fileDelta === 0 || rankDelta === 0) &&
      (fileDelta !== 0 || rankDelta !== 0) &&
      pathClear(state, from, square, Math.sign(fileDelta), Math.sign(rankDelta))
    );
  }
  if (piece.type === "q") {
    if (Math.abs(fileDelta) === Math.abs(rankDelta) && fileDelta !== 0) {
      return pathClear(state, from, square, Math.sign(fileDelta), Math.sign(rankDelta));
    }

    return (
      (fileDelta === 0 || rankDelta === 0) &&
      (fileDelta !== 0 || rankDelta !== 0) &&
      pathClear(state, from, square, Math.sign(fileDelta), Math.sign(rankDelta))
    );
  }
  if (piece.type === "k") {
    return Math.abs(fileDelta) <= 1 && Math.abs(rankDelta) <= 1 && (fileDelta !== 0 || rankDelta !== 0);
  }

  return false;
}

function pathClear(state: BoardState, from: number, to: number, fileStep: number, rankStep: number): boolean {
  let file = fileOf(from) + fileStep;
  let rank = rankOf(from) + rankStep;

  while (true) {
    const square = indexFor(file, rank);

    if (square === null) {
      return false;
    }
    if (square === to) {
      return true;
    }
    if (state.squares[square]) {
      return false;
    }

    file += fileStep;
    rank += rankStep;
  }
}

function stateToFen(state: BoardState): string {
  const ranks: string[] = [];

  for (let rank = 8; rank >= 1; rank -= 1) {
    let text = "";
    let empty = 0;

    for (let file = 0; file < 8; file += 1) {
      const square = indexFor(file, rank);
      const piece = square === null ? null : state.squares[square];

      if (!piece) {
        empty += 1;
        continue;
      }

      if (empty > 0) {
        text += String(empty);
        empty = 0;
      }

      text += piece.color === "white" ? piece.type.toUpperCase() : piece.type;
    }

    if (empty > 0) {
      text += String(empty);
    }

    ranks.push(text);
  }

  return `${ranks.join("/")} ${state.sideToMove === "white" ? "w" : "b"} ${castlingToFen(state.castling)} ${
    state.enPassant === null ? "-" : indexToSquare(state.enPassant)
  } ${state.halfmoveClock} ${state.fullmoveNumber}`;
}

function castlingToFen(castling: CastlingRights): string {
  const text = `${castling.whiteKingSide ? "K" : ""}${castling.whiteQueenSide ? "Q" : ""}${castling.blackKingSide ? "k" : ""}${
    castling.blackQueenSide ? "q" : ""
  }`;

  return text || "-";
}
