import type { PlayerColor } from "./types";

export type PieceType = "pawn" | "knight" | "bishop" | "rook" | "queen" | "king";

export interface BoardPiece {
  type: PieceType;
  color: PlayerColor;
}

export type BoardSquare = BoardPiece | null;

const pieceMap: Record<string, PieceType> = {
  p: "pawn",
  n: "knight",
  b: "bishop",
  r: "rook",
  q: "queen",
  k: "king",
};

export function parseFenBoard(fen: string): BoardSquare[] {
  const boardPart = fen.split(" ")[0];

  if (!boardPart) {
    return Array.from({ length: 64 }, () => null);
  }

  const squares: BoardSquare[] = [];

  for (const char of boardPart) {
    if (char === "/") {
      continue;
    }

    const emptyCount = Number(char);

    if (Number.isInteger(emptyCount) && emptyCount > 0) {
      for (let i = 0; i < emptyCount; i += 1) {
        squares.push(null);
      }

      continue;
    }

    const lower = char.toLowerCase();
    const type = pieceMap[lower];

    if (!type) {
      continue;
    }

    squares.push({
      type,
      color: char === lower ? "black" : "white",
    });
  }

  while (squares.length < 64) {
    squares.push(null);
  }

  return squares.slice(0, 64);
}

export function squareNameFromIndex(index: number): string {
  const file = index % 8;
  const rankFromTop = Math.floor(index / 8);
  const fileName = String.fromCharCode("a".charCodeAt(0) + file);
  const rankName = String(8 - rankFromTop);

  return `${fileName}${rankName}`;
}
