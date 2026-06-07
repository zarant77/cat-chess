import { useEffect, useMemo, useState } from "preact/hooks";
import { createGame, getGame, joinGame, listGames, listMoves, makeMove, resignGame } from "./api";
import { getOrCreateDeviceSecret } from "./device";
import type { GameDto, MoveDto, PlayerColor } from "./types";
import { ChessBoard } from "./components/ChessBoard";
import type { BoardPiece } from "./chessFen";

type Screen = "home" | "join" | "my-games" | "game";

export function App() {
  const [screen, setScreen] = useState<Screen>("home");
  const [deviceSecret, setDeviceSecret] = useState<string | null>(null);
  const [games, setGames] = useState<GameDto[]>([]);
  const [currentGame, setCurrentGame] = useState<GameDto | null>(null);
  const [moves, setMoves] = useState<MoveDto[]>([]);
  const [inviteCodeInput, setInviteCodeInput] = useState("");
  const [selectedSquare, setSelectedSquare] = useState<string | null>(null);
  const [statusText, setStatusText] = useState("Loading device...");
  const [isMenuOpen, setIsMenuOpen] = useState(false);
  const [isMovesOpen, setIsMovesOpen] = useState(false);

  const currentGameId = currentGame?.id ?? null;

  useEffect(() => {
    getOrCreateDeviceSecret()
      .then((secret) => {
        setDeviceSecret(secret);
        setStatusText("Ready");
      })
      .catch((error: unknown) => {
        setStatusText(error instanceof Error ? error.message : "device_error");
      });
  }, []);

  useEffect(() => {
    if (!deviceSecret || !currentGameId || screen !== "game") {
      return;
    }

    const timer = window.setInterval(() => {
      void loadGame(currentGameId, false);
    }, 4000);

    return () => window.clearInterval(timer);
  }, [deviceSecret, currentGameId, screen]);

  const gameStatusText = useMemo(() => {
    if (!currentGame) {
      return statusText;
    }

    if (currentGame.status === "waiting_for_black") {
      return currentGame.inviteCode ? `Invite code: ${currentGame.inviteCode}` : "Waiting for opponent";
    }

    if (currentGame.status === "finished") {
      if (currentGame.result === "draw") {
        return "Draw";
      }

      const winner: PlayerColor | null = currentGame.result === "white_won" ? "white" : currentGame.result === "black_won" ? "black" : null;

      if (!winner || !currentGame.yourColor) {
        return "Game finished";
      }

      return winner === currentGame.yourColor ? "You won" : "You lost";
    }

    if (currentGame.yourColor === currentGame.sideToMove) {
      return "Your move";
    }

    return "Waiting for opponent";
  }, [currentGame, statusText]);

  async function refreshGames(secret = deviceSecret): Promise<void> {
    if (!secret) {
      setStatusText("Device is not ready yet");
      return;
    }

    const response = await listGames(secret);
    setGames(response.games);
  }

  async function loadGame(gameId: number, showLoading = true): Promise<void> {
    if (!deviceSecret) {
      setStatusText("Device is not ready yet");
      return;
    }

    try {
      if (showLoading) {
        setStatusText("Opening game...");
      }

      const [game, moveResponse] = await Promise.all([getGame(deviceSecret, gameId), listMoves(deviceSecret, gameId)]);

      setCurrentGame(game);
      setMoves(moveResponse.moves);
      setSelectedSquare(null);

      if (showLoading) {
        setStatusText("Game opened");
      }
    } catch (error: unknown) {
      setStatusText(error instanceof Error ? error.message : "open_game_error");
    }
  }

  async function openGame(gameId: number): Promise<void> {
    await loadGame(gameId);
    setScreen("game");
    setIsMenuOpen(false);
    setIsMovesOpen(false);
  }

  async function handleCreateGame(): Promise<void> {
    if (!deviceSecret) {
      setStatusText("Device is not ready yet");
      return;
    }

    try {
      setStatusText("Creating game...");

      const game = await createGame(deviceSecret);

      setCurrentGame(game);
      setMoves([]);
      setSelectedSquare(null);
      setScreen("game");
      setIsMenuOpen(false);
      setIsMovesOpen(false);
      setStatusText("Game created");
    } catch (error: unknown) {
      setStatusText(error instanceof Error ? error.message : "create_game_error");
    }
  }

  async function handleJoinGame(): Promise<void> {
    if (!deviceSecret) {
      setStatusText("Device is not ready yet");
      return;
    }

    if (!inviteCodeInput.trim()) {
      setStatusText("Enter invite code");
      return;
    }

    try {
      setStatusText("Joining game...");

      const game = await joinGame(deviceSecret, inviteCodeInput);

      setCurrentGame(game);
      setMoves([]);
      setSelectedSquare(null);
      setScreen("game");
      setIsMenuOpen(false);
      setIsMovesOpen(false);
      setStatusText("Joined game");
    } catch (error: unknown) {
      setStatusText(error instanceof Error ? error.message : "join_game_error");
    }
  }

  async function handleMyGames(): Promise<void> {
    try {
      setStatusText("Loading games...");
      await refreshGames();
      setScreen("my-games");
      setStatusText("Games loaded");
    } catch (error: unknown) {
      setStatusText(error instanceof Error ? error.message : "list_games_error");
    }
  }

  async function handleResign(): Promise<void> {
    if (!deviceSecret || !currentGame) {
      setStatusText("Game is not ready yet");
      return;
    }

    try {
      const game = await resignGame(deviceSecret, currentGame.id);

      setCurrentGame(game);
      setIsMenuOpen(false);
      setStatusText("Game resigned");
    } catch (error: unknown) {
      setStatusText(error instanceof Error ? error.message : "resign_error");
    }
  }

  async function handleSquareClick(square: string, piece: BoardPiece | null): Promise<void> {
    if (!currentGame || !deviceSecret) {
      return;
    }

    if (currentGame.status !== "active") {
      setStatusText("Game is not active");
      return;
    }

    if (currentGame.yourColor !== currentGame.sideToMove) {
      setStatusText("Not your turn");
      return;
    }

    if (!selectedSquare) {
      if (!piece) {
        return;
      }

      if (piece.color !== currentGame.yourColor) {
        setStatusText("Select your piece");
        return;
      }

      setSelectedSquare(square);
      setStatusText(`Selected ${square}`);
      return;
    }

    const uci = `${selectedSquare}${square}`;
    setSelectedSquare(null);

    try {
      const game = await makeMove(deviceSecret, currentGame.id, uci);
      const moveResponse = await listMoves(deviceSecret, currentGame.id);

      setCurrentGame(game);
      setMoves(moveResponse.moves);
      setStatusText(`Move: ${uci}`);
    } catch (error: unknown) {
      setStatusText(error instanceof Error ? error.message : "move_error");
    }
  }

  function goHome(): void {
    setScreen("home");
    setIsMenuOpen(false);
    setIsMovesOpen(false);
    setSelectedSquare(null);
  }

  return (
    <main class={screen === "game" ? "app-shell is-game-screen" : "app-shell"}>
      {screen !== "game" && (
        <>
          <header class="app-header">
            <div>
              <h1>Cat Chess</h1>
              <p>No accounts. Just chess.</p>
            </div>

            <div class="device-pill">{deviceSecret ? "Device ready" : "No device"}</div>
          </header>

          <section class="status-line">{statusText}</section>
        </>
      )}

      {screen === "home" && (
        <section class="panel menu-panel">
          <button type="button" onClick={() => void handleCreateGame()}>
            Create Online Game
          </button>

          <button
            type="button"
            onClick={() => {
              setScreen("join");
              setStatusText("Enter invite code");
            }}
          >
            Join Online Game
          </button>

          <button type="button" onClick={() => void handleMyGames()}>
            My Games
          </button>
        </section>
      )}

      {screen === "join" && (
        <section class="panel join-panel">
          <h2>Join Game</h2>

          <input value={inviteCodeInput} onInput={(event) => setInviteCodeInput(event.currentTarget.value)} placeholder="Invite code" />

          <div class="row">
            <button type="button" onClick={goHome}>
              Back
            </button>

            <button type="button" onClick={() => void handleJoinGame()}>
              Join
            </button>
          </div>
        </section>
      )}

      {screen === "my-games" && (
        <section class="panel my-games-panel">
          <h2>My Games</h2>

          <div class="game-list">
            {games.length === 0 && <p>No games yet.</p>}

            {games.map((game) => (
              <button key={game.id} type="button" class="game-card" onClick={() => void openGame(game.id)}>
                <strong>Game #{game.id}</strong>
                <span>{formatGameCardStatus(game)}</span>
              </button>
            ))}
          </div>

          <div class="row">
            <button type="button" onClick={goHome}>
              Back
            </button>

            <button type="button" onClick={() => void handleMyGames()}>
              Reload
            </button>
          </div>
        </section>
      )}

      {screen === "game" && currentGame && (
        <section class="game-screen">
          <header class="game-topbar">
            <button type="button" class="icon-button" onClick={() => setIsMenuOpen((value) => !value)}>
              ☰
            </button>

            <div class="game-title">
              <strong>Game #{currentGame.id}</strong>
              <span>{formatTopbarSide(currentGame)}</span>
            </div>

            <button type="button" class="topbar-button" onClick={() => setIsMovesOpen((value) => !value)}>
              Moves {moves.length}
            </button>
          </header>

          {isMenuOpen && (
            <section class="game-menu panel">
              <button type="button" onClick={goHome}>
                Home
              </button>

              <button type="button" onClick={() => void handleMyGames()}>
                My Games
              </button>

              {currentGame.inviteCode && currentGame.status === "waiting_for_black" && (
                <button
                  type="button"
                  onClick={() => {
                    void navigator.clipboard?.writeText(currentGame.inviteCode ?? "");
                    setStatusText("Invite code copied");
                    setIsMenuOpen(false);
                  }}
                >
                  Copy Invite
                </button>
              )}

              {currentGame.status === "active" && (
                <button type="button" class="danger-button" onClick={() => void handleResign()}>
                  Resign
                </button>
              )}
            </section>
          )}

          <section class="game-board-wrap">
            <ChessBoard
              fen={currentGame.boardFen}
              selectedSquare={selectedSquare}
              perspective={currentGame.yourColor ?? "white"}
              onSquareClick={(square, piece) => {
                void handleSquareClick(square, piece);
              }}
            />
          </section>

          <section class={isMovesOpen ? "moves-drawer is-open" : "moves-drawer"}>
            <div class="moves-drawer-header">
              <strong>Moves</strong>
              <button type="button" class="small-button" onClick={() => setIsMovesOpen(false)}>
                Close
              </button>
            </div>

            {moves.length === 0 && <p>No moves yet.</p>}

            <ol>
              {moves.map((move) => (
                <li key={move.id}>
                  {move.moveIndex + 1}. {move.color} {move.uci}
                </li>
              ))}
            </ol>
          </section>

          <footer class="game-statusbar">{gameStatusText}</footer>
        </section>
      )}
    </main>
  );
}

function formatTopbarSide(game: GameDto): string {
  if (game.status === "waiting_for_black") {
    return game.inviteCode ? `Invite ${game.inviteCode}` : "Waiting";
  }

  if (game.status === "finished") {
    return formatGameResult(game);
  }

  if (!game.yourColor) {
    return "Playing";
  }

  return game.yourColor === "white" ? "White side" : "Black side";
}

function formatGameCardStatus(game: GameDto): string {
  if (game.status === "waiting_for_black") {
    return game.inviteCode ? `Waiting · ${game.inviteCode}` : "Waiting";
  }

  if (game.status === "finished") {
    return formatGameResult(game);
  }

  return game.yourColor === game.sideToMove ? "Your move" : "Opponent move";
}

function formatGameResult(game: GameDto): string {
  if (game.result === "draw") {
    return "Draw";
  }

  const winner = game.result === "white_won" ? "White" : game.result === "black_won" ? "Black" : null;

  if (!winner) {
    return "Finished";
  }

  return `${winner} won`;
}
