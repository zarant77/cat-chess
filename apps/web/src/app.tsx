import { useEffect, useMemo, useRef, useState } from "preact/hooks";
import { createGame, getGame, joinGame, listGames, listMoves, makeMove, resignGame } from "./api";
import { getOrCreateDeviceSecret } from "./device";
import type { GameDto, MoveDto, PlayerColor } from "./types";
import { ChessBoard } from "./components/ChessBoard";
import { parseFenBoard } from "./chessFen";
import type { BoardPiece } from "./chessFen";
import { buildMoveLog, checkedKingSquare, isPromotionTarget, legalMovesForSquare, uciPromotionChar } from "./chessRulesWeb";
import type { MoveLogEntry, PromotionPiece } from "./chessRulesWeb";

type Screen = "home" | "online" | "create" | "join" | "my-games" | "settings" | "game";
type WebLocale = "en" | "uk";
type WebSoundId = "move" | "capture" | "check" | "checkmate" | "illegal" | "menu_select" | "menu_back" | "game_created" | "game_joined";

type WebSettings = {
  soundsEnabled: boolean;
  musicEnabled: boolean;
  showMoveHints: boolean;
  locale: WebLocale;
};

const DEBUG_SYNC = import.meta.env.VITE_CAT_CHESS_DEBUG_SYNC === "1";
const WEB_SETTINGS_KEY = "cat-chess-settings";
const START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
let webMusicContext: AudioContext | null = null;
let webMusicTimer: number | null = null;
let webMusicPlaying = false;

type TextKey =
  | "local_game"
  | "online"
  | "my_games"
  | "settings"
  | "settings_short"
  | "back"
  | "sounds"
  | "music"
  | "hints"
  | "language"
  | "moves"
  | "captured"
  | "promotion"
  | "no_moves"
  | "your_move"
  | "opponent_turn"
  | "waiting_for_opponent"
  | "game_created"
  | "draw"
  | "finished"
  | "you_won"
  | "you_lost"
  | "check";

const TEXT: Record<WebLocale, Record<TextKey, string>> = {
  en: {
    local_game: "Local Game",
    online: "Online",
    my_games: "My Games",
    settings: "Settings",
    settings_short: "Set",
    back: "Back",
    sounds: "Sounds",
    music: "Music",
    hints: "Hints",
    language: "Language",
    moves: "Moves",
    captured: "Captured",
    promotion: "Promotion",
    no_moves: "No moves yet",
    your_move: "Your move",
    opponent_turn: "Opponent turn",
    waiting_for_opponent: "Waiting for opponent",
    game_created: "Game created",
    draw: "Draw",
    finished: "Game finished",
    you_won: "You won",
    you_lost: "You lost",
    check: "Check",
  },
  uk: {
    local_game: "Локальна гра",
    online: "Онлайн",
    my_games: "Мої ігри",
    settings: "Налаштування",
    settings_short: "Опції",
    back: "Назад",
    sounds: "Звуки",
    music: "Музика",
    hints: "Підказки",
    language: "Мова",
    moves: "Ходи",
    captured: "Захоплено",
    promotion: "Перетворення",
    no_moves: "Ходів ще немає",
    your_move: "Ваш хід",
    opponent_turn: "Хід суперника",
    waiting_for_opponent: "Чекаємо суперника",
    game_created: "Гру створено",
    draw: "Нічия",
    finished: "Гру завершено",
    you_won: "Ви перемогли",
    you_lost: "Ви програли",
    check: "Шах",
  },
};

function localizedText(locale: WebLocale, key: TextKey): string {
  return TEXT[locale][key];
}

export function App() {
  const [screen, setScreen] = useState<Screen>("home");
  const [deviceSecret, setDeviceSecret] = useState<string | null>(null);
  const [games, setGames] = useState<GameDto[]>([]);
  const [currentGame, setCurrentGame] = useState<GameDto | null>(null);
  const [moves, setMoves] = useState<MoveDto[]>([]);
  const [inviteCodeInput, setInviteCodeInput] = useState("");
  const [selectedSquare, setSelectedSquare] = useState<string | null>(null);
  const [statusText, setStatusText] = useState("Loading device...");
  const [soundsEnabled, setSoundsEnabled] = useState(true);
  const [musicEnabled, setMusicEnabled] = useState(true);
  const [showMoveHints, setShowMoveHints] = useState(true);
  const [webLocale, setWebLocale] = useState<WebLocale>("en");
  const [isMenuOpen, setIsMenuOpen] = useState(false);
  const [isMovesOpen, setIsMovesOpen] = useState(false);
  const [isAnimatingMove, setIsAnimatingMove] = useState(false);
  const [promotionMove, setPromotionMove] = useState<{ from: string; to: string } | null>(null);
  const [settingsReturnScreen, setSettingsReturnScreen] = useState<Screen>("home");
  const currentGameRef = useRef<GameDto | null>(null);
  const pollInFlightRef = useRef(false);
  const latestGameRequestRef = useRef(0);

  const currentGameId = currentGame?.id ?? null;
  const text = (key: TextKey) => localizedText(webLocale, key);

  useEffect(() => {
    currentGameRef.current = currentGame;
  }, [currentGame]);

  useEffect(() => {
    const settings = loadWebSettings();
    setSoundsEnabled(settings.soundsEnabled);
    setMusicEnabled(settings.musicEnabled);
    setShowMoveHints(settings.showMoveHints);
    setWebLocale(settings.locale);

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
    saveWebSettings({ soundsEnabled, musicEnabled, showMoveHints, locale: webLocale });
    if (musicEnabled) {
      playWebMusic();
    } else {
      stopWebMusic();
    }
  }, [soundsEnabled, musicEnabled, showMoveHints, webLocale]);

  function playSound(id: WebSoundId): void {
    playWebSound(id, soundsEnabled);
  }

  useEffect(() => {
    if (!deviceSecret || !currentGameId || screen !== "game" || currentGame?.status === "finished") {
      return;
    }

    const timer = window.setInterval(() => {
      void pollGame(currentGameId);
    }, 1500);

    return () => window.clearInterval(timer);
  }, [deviceSecret, currentGameId, currentGame?.status, screen]);

  const gameStatusText = useMemo(() => {
    if (!currentGame) {
      return statusText;
    }

    if (currentGame.status === "waiting_for_black") {
      return currentGame.inviteCode ? text("game_created") : text("waiting_for_opponent");
    }

    if (currentGame.status === "finished") {
      if (currentGame.result === "draw") {
        return text("draw");
      }

      const winner: PlayerColor | null = currentGame.result === "white_won" ? "white" : currentGame.result === "black_won" ? "black" : null;

      if (!winner || !currentGame.yourColor) {
        return text("finished");
      }

      return winner === currentGame.yourColor ? text("you_won") : text("you_lost");
    }

    if (checkedKingSquare(currentGame.boardFen)) {
      return text("check");
    }

    if (currentGame.yourColor === currentGame.sideToMove) {
      return text("your_move");
    }

    return text("opponent_turn");
  }, [currentGame, statusText, webLocale]);

  const moveLog = useMemo<MoveLogEntry[]>(() => {
    if (!currentGame) {
      return [];
    }
    return buildMoveLog(START_FEN, moves);
  }, [currentGame?.id, moves]);

  const legalTargets = useMemo(() => {
    if (!currentGame || !selectedSquare || !showMoveHints || currentGame.status !== "active" || currentGame.yourColor !== currentGame.sideToMove) {
      return [];
    }
    return legalMovesForSquare(currentGame.boardFen, selectedSquare).map((move) => ({ square: move.to, capture: move.capture }));
  }, [currentGame?.boardFen, currentGame?.status, currentGame?.yourColor, currentGame?.sideToMove, selectedSquare, showMoveHints]);

  const checkSquare = useMemo(() => (currentGame ? checkedKingSquare(currentGame.boardFen) : null), [currentGame?.boardFen]);

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
      latestGameRequestRef.current += 1;

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

  async function pollGame(gameId: number): Promise<void> {
    if (!deviceSecret || pollInFlightRef.current) {
      return;
    }

    pollInFlightRef.current = true;
    const requestId = latestGameRequestRef.current + 1;
    latestGameRequestRef.current = requestId;

    try {
      const game = await getGame(deviceSecret, gameId);
      const previousGame = currentGameRef.current;

      if (previousGame?.id !== gameId) {
        return;
      }

      const changed =
        !previousGame ||
        previousGame.id !== game.id ||
        previousGame.updatedAt !== game.updatedAt ||
        previousGame.boardFen !== game.boardFen ||
        previousGame.sideToMove !== game.sideToMove ||
        previousGame.status !== game.status;

      if (requestId !== latestGameRequestRef.current) {
        return;
      }

      if (!changed) {
        return;
      }

      if (DEBUG_SYNC) {
        console.debug("[cat-chess] poll", {
          gameId,
          previousBoardFen: previousGame?.boardFen,
          boardFen: game.boardFen,
          yourColor: game.yourColor,
          sideToMove: game.sideToMove,
        });
      }

      setCurrentGame(game);
      setSelectedSquare(null);

      const moveResponse = await listMoves(deviceSecret, gameId);
      if (requestId === latestGameRequestRef.current) {
        setMoves(moveResponse.moves);
      }
    } catch {
      // Polling is best-effort; explicit user actions still report errors.
    } finally {
      pollInFlightRef.current = false;
    }
  }

  async function openGame(gameId: number): Promise<void> {
    await loadGame(gameId);
    playSound("menu_select");
    setScreen("game");
    setIsMenuOpen(false);
    setIsMovesOpen(false);
  }

  async function handleCreateGame(): Promise<void> {
    playSound("menu_select");
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
      playSound("game_created");
      setScreen("game");
      setIsMenuOpen(false);
      setIsMovesOpen(false);
      setStatusText("Game created");
    } catch (error: unknown) {
      setStatusText(error instanceof Error ? error.message : "create_game_error");
    }
  }

  async function handleJoinGame(): Promise<void> {
    playSound("menu_select");
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
      playSound("game_joined");
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
      playSound("menu_select");
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
      playSound("illegal");
      return;
    }

    if (currentGame.yourColor !== currentGame.sideToMove) {
      setStatusText("Not your turn");
      playSound("illegal");
      return;
    }

    if (!selectedSquare) {
      if (!piece) {
        playSound("illegal");
        return;
      }

      if (piece.color !== currentGame.yourColor) {
        setStatusText("Select your piece");
        playSound("illegal");
        return;
      }

      setSelectedSquare(square);
      setStatusText(`Selected ${square}`);
      return;
    }

    const from = selectedSquare;
    if (piece?.color === currentGame.yourColor) {
      setSelectedSquare(square);
      setStatusText(`Selected ${square}`);
      return;
    }

    setSelectedSquare(null);
    const isLegalPromotion = legalMovesForSquare(currentGame.boardFen, from).some((move) => move.to === square && !!move.promotion);
    if (isPromotionTarget(currentGame.boardFen, from, square) && isLegalPromotion) {
      setPromotionMove({ from, to: square });
      return;
    }

    await submitMove(`${from}${square}`, piece);
  }

  async function submitMove(uci: string, targetPiece: BoardPiece | null): Promise<void> {
    if (!currentGame || !deviceSecret) {
      return;
    }

    try {
      latestGameRequestRef.current += 1;

      const game = await makeMove(deviceSecret, currentGame.id, uci);
      const moveResponse = await listMoves(deviceSecret, currentGame.id);

      if (DEBUG_SYNC) {
        console.debug("[cat-chess] move", {
          gameId: currentGame.id,
          submittedMove: uci,
          previousBoardFen: currentGame.boardFen,
          responseBoardFen: game.boardFen,
          yourColor: game.yourColor,
          sideToMove: game.sideToMove,
        });
      }

      setCurrentGame(game);
      setMoves(moveResponse.moves);
      setStatusText(`Move: ${uci}`);
      playSound(soundForMoveResult(game, targetPiece));
    } catch (error: unknown) {
      setStatusText(error instanceof Error ? error.message : "move_error");
      playSound("illegal");
    }
  }

  async function choosePromotion(piece: PromotionPiece): Promise<void> {
    if (!promotionMove) {
      return;
    }
    const target = currentGame ? parseCurrentTarget(currentGame.boardFen, promotionMove.to) : null;
    const uci = `${promotionMove.from}${promotionMove.to}${uciPromotionChar(piece)}`;
    setPromotionMove(null);
    await submitMove(uci, target);
  }

  function goHome(): void {
    playSound("menu_back");
    setScreen("home");
    setIsMenuOpen(false);
    setIsMovesOpen(false);
    setSelectedSquare(null);
  }

  const showJoinStatus = screen === "join" && !["Ready", "Enter invite code", "Loading device..."].includes(statusText);

  return (
    <main class={screen === "game" ? "app-shell is-game-screen" : "app-shell"}>
      {screen === "home" && (
        <section class="screen-column home-screen">
          <header class="brand-block">
            <div class="web-main-logo" aria-label="Cat Chess">
              <span>CAT</span>
              <strong>CHESS</strong>
            </div>
          </header>

          <nav class="menu-stack">
            <button
              type="button"
              class="button button-primary"
              onClick={() => {
                playSound("menu_select");
                setStatusText("Local game is available in the Android client.");
              }}
            >
              {text("local_game")}
            </button>

            <button
              type="button"
              class="button button-primary"
              onClick={() => {
                playSound("menu_select");
                setScreen("online");
              }}
            >
              {text("online")}
            </button>

            <button type="button" class="button button-secondary" onClick={() => void handleMyGames()}>
              {text("my_games")}
            </button>

            <button
              type="button"
              class="button button-secondary"
              onClick={() => {
                playSound("menu_select");
                setSettingsReturnScreen("home");
                setScreen("settings");
              }}
            >
              {text("settings")}
            </button>
          </nav>

          <section class="home-note">{statusText}</section>
        </section>
      )}

      {screen === "online" && (
        <section class="screen-column">
          <ScreenTitle title="ONLINE" />

          <nav class="menu-stack">
            <button
              type="button"
              class="button button-primary"
              onClick={() => {
                playSound("menu_select");
                setScreen("create");
              }}
            >
              Create Game
            </button>

            <button
              type="button"
              class="button button-primary"
              onClick={() => {
                playSound("menu_select");
                setScreen("join");
                setStatusText("Enter invite code");
              }}
            >
              Join Game
            </button>

            <button type="button" class="button button-compact" onClick={goHome}>
              Back
            </button>
          </nav>
        </section>
      )}

      {screen === "create" && (
        <section class="screen-column">
          <ScreenTitle title="CREATE GAME" />

          <section class="status-card">
            <span class="eyebrow">SERVER</span>
            <strong>{deviceSecret ? "SERVER READY" : "CONNECTING"}</strong>
            <span>{statusText}</span>
          </section>

          <div class="menu-stack">
            <button type="button" class="button button-primary" disabled={!deviceSecret} onClick={() => void handleCreateGame()}>
              Create Game
            </button>

            <button
              type="button"
              class="button button-compact"
              onClick={() => {
                playSound("menu_back");
                setScreen("online");
              }}
            >
              Back
            </button>
          </div>
        </section>
      )}

      {screen === "join" && (
        <section class="screen-column">
          <ScreenTitle title="JOIN GAME" />

          <label class="input-card">
            <span>INVITE CODE</span>
            <input
              value={inviteCodeInput}
              onInput={(event) => setInviteCodeInput(event.currentTarget.value.trim().toUpperCase())}
              placeholder="CAT-PAW"
              autoComplete="off"
              spellCheck={false}
            />
          </label>

          {showJoinStatus && (
            <section class="status-card status-card-small">
              <strong>{statusText}</strong>
            </section>
          )}

          <div class="menu-stack">
            <button type="button" class="button button-primary" disabled={!inviteCodeInput.trim()} onClick={() => void handleJoinGame()}>
              Join Game
            </button>

            <button
              type="button"
              class="button button-compact"
              onClick={() => {
                playSound("menu_back");
                setScreen("online");
              }}
            >
              Back
            </button>
          </div>
        </section>
      )}

      {screen === "my-games" && (
        <section class="screen-column my-games-panel">
          <ScreenTitle title="MY GAMES" />

          <div class="game-list">
            {games.length === 0 && (
              <section class="empty-state">
                <strong>No games yet</strong>
                <span>Create online game to start</span>
              </section>
            )}

            {games.map((game) => (
              <button key={game.id} type="button" class="game-card" onClick={() => void openGame(game.id)}>
                <strong>Game #{game.id}</strong>
                <span>{formatGameCardStatus(game)}</span>
                <small>
                  {formatPlayerColor(game.yourColor)} · {game.status.replace(/_/g, " ")}
                </small>
              </button>
            ))}
          </div>

          <div class="bottom-actions">
            <button type="button" class="button button-compact" onClick={goHome}>
              Back
            </button>

            <button type="button" class="button button-secondary" onClick={() => void handleMyGames()}>
              Reload
            </button>
          </div>
        </section>
      )}

      {screen === "settings" && (
        <section class="screen-column">
          <ScreenTitle title={text("settings")} />

          <section class="settings-card">
            <SettingsCheckbox label={text("sounds")} checked={soundsEnabled} onToggle={() => setSoundsEnabled((value) => !value)} />
            <SettingsCheckbox label={text("music")} checked={musicEnabled} onToggle={() => setMusicEnabled((value) => !value)} />
            <SettingsCheckbox label={text("hints")} checked={showMoveHints} onToggle={() => setShowMoveHints((value) => !value)} />

            <strong>{text("language")}</strong>
            <div class="settings-row flag-row">
              <button type="button" class={webLocale === "uk" ? "flag-button is-selected" : "flag-button"} onClick={() => setWebLocale("uk")}>
                <span class="flag-icon flag-uk" aria-hidden="true" />
              </button>
              <button type="button" class={webLocale === "en" ? "flag-button is-selected" : "flag-button"} onClick={() => setWebLocale("en")}>
                <span class="flag-icon flag-en" aria-hidden="true" />
              </button>
            </div>
          </section>

          <button
            type="button"
            class="button button-compact"
            onClick={() => {
              playSound("menu_back");
              setScreen(settingsReturnScreen);
            }}
          >
            {text("back")}
          </button>
        </section>
      )}

      {screen === "game" && currentGame && (
        <section class="game-screen">
          <header class="game-topbar">
            <button type="button" class="button button-compact topbar-back" onClick={goHome}>
              {text("back")}
            </button>

            <div class="game-title">
              <strong>{gameStatusText}</strong>
              <span>Game #{currentGame.id} · {formatTopbarSide(currentGame)}</span>
            </div>

            <button
              type="button"
              class="button button-compact topbar-button"
              onClick={() => {
                setSettingsReturnScreen("game");
                setScreen("settings");
              }}
            >
              {text("settings_short")}
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
            {currentGame.status === "waiting_for_black" && currentGame.inviteCode ? (
              <section class="invite-code-card">
                <span>INVITE CODE</span>
                <strong>{currentGame.inviteCode}</strong>
                <small>WAITING FOR OPPONENT</small>
                <button
                  type="button"
                  class="button button-secondary"
                  onClick={() => {
                    void navigator.clipboard?.writeText(currentGame.inviteCode ?? "");
                    setStatusText("Invite code copied");
                  }}
                >
                  Copy Code
                </button>
              </section>
            ) : (
              <ChessBoard
                fen={currentGame.boardFen}
                selectedSquare={selectedSquare}
                perspective={currentGame.yourColor ?? "white"}
                lastMove={currentGame.lastMove}
                legalTargets={legalTargets}
                checkSquare={checkSquare}
                disabled={isAnimatingMove || currentGame.status !== "active" || currentGame.yourColor !== currentGame.sideToMove}
                onAnimationChange={setIsAnimatingMove}
                onSquareClick={(square, piece) => {
                  void handleSquareClick(square, piece);
                }}
              />
            )}
          </section>

          <MoveAndCapturePanel entries={moveLog} locale={webLocale} />

          {promotionMove && (
            <PromotionPicker
              locale={webLocale}
              color={currentGame.yourColor ?? "white"}
              onChoose={(piece) => {
                void choosePromotion(piece);
              }}
            />
          )}

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

function SettingsCheckbox(props: { label: string; checked: boolean; onToggle: () => void }) {
  return (
    <button type="button" class="settings-check-row" onClick={props.onToggle}>
      <span class={props.checked ? "web-checkbox is-checked" : "web-checkbox"} aria-hidden="true" />
      <strong>{props.label}</strong>
    </button>
  );
}

function PromotionPicker(props: { locale: WebLocale; color: PlayerColor; onChoose: (piece: PromotionPiece) => void }) {
  const pieces: PromotionPiece[] = ["queen", "rook", "bishop", "knight"];
  return (
    <section class="modal-scrim">
      <div class="promotion-modal">
        <strong>{localizedText(props.locale, "promotion")}</strong>
        <div class="promotion-grid">
          {pieces.map((piece) => (
            <button key={piece} type="button" class="promotion-choice" onClick={() => props.onChoose(piece)}>
              <PieceIcon piece={{ type: piece, color: props.color }} />
            </button>
          ))}
        </div>
      </div>
    </section>
  );
}

function MoveAndCapturePanel(props: { entries: MoveLogEntry[]; locale: WebLocale }) {
  const rows = groupMoveRows(props.entries).slice(-6);
  const capturedWhite = props.entries.map((entry) => entry.captured).filter((piece): piece is BoardPiece => !!piece && piece.color === "white");
  const capturedBlack = props.entries.map((entry) => entry.captured).filter((piece): piece is BoardPiece => !!piece && piece.color === "black");

  return (
    <section class="move-capture-panel">
      <div class="move-log-compact">
        <strong>{localizedText(props.locale, "moves")}</strong>
        {rows.length === 0 ? <span class="muted">{localizedText(props.locale, "no_moves")}</span> : rows.map((row) => <span key={row}>{row}</span>)}
      </div>
      <div class="captured-compact">
        <strong>{localizedText(props.locale, "captured")}</strong>
        <div class="captured-row">{capturedWhite.map((piece, index) => <PieceIcon key={`white-${piece.type}-${index}`} piece={piece} />)}</div>
        <div class="captured-row">{capturedBlack.map((piece, index) => <PieceIcon key={`black-${piece.type}-${index}`} piece={piece} />)}</div>
      </div>
    </section>
  );
}

function groupMoveRows(entries: MoveLogEntry[]): string[] {
  const rows: string[] = [];
  for (let index = 0; index < entries.length; index += 1) {
    const entry = entries[index];
    if (entry.color !== "white") {
      continue;
    }
    const black = entries[index + 1]?.moveNumber === entry.moveNumber && entries[index + 1]?.color === "black" ? entries[index + 1] : null;
    rows.push(`${entry.moveNumber}. ${entry.display}${black ? ` ${black.display}` : ""}`);
  }
  return rows;
}

function PieceIcon({ piece }: { piece: BoardPiece }) {
  const pieceColumn: Record<BoardPiece["type"], number> = {
    pawn: 0,
    knight: 1,
    bishop: 2,
    rook: 3,
    queen: 4,
    king: 5,
  };
  const col = pieceColumn[piece.type];
  const row = piece.color === "white" ? 0 : 1;
  return <span class="piece-sprite piece-icon" style={{ backgroundPosition: `${col * 20}% ${row * 100}%` }} />;
}

function ScreenTitle(props: { title: string }) {
  return (
    <header class="screen-title">
      <h1>{props.title}</h1>
      <div class="cat-mark" aria-hidden="true" />
    </header>
  );
}

function formatPlayerColor(color: PlayerColor | null): string {
  if (color === "white") {
    return "White";
  }
  if (color === "black") {
    return "Black";
  }
  return "No color";
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

function loadWebSettings(): WebSettings {
  try {
    const raw = window.localStorage.getItem(WEB_SETTINGS_KEY);
    if (!raw) {
      return { soundsEnabled: true, musicEnabled: true, showMoveHints: true, locale: "en" };
    }

    const parsed = JSON.parse(raw) as Partial<WebSettings>;
    return {
      soundsEnabled: parsed.soundsEnabled !== false,
      musicEnabled: parsed.musicEnabled !== false,
      showMoveHints: parsed.showMoveHints !== false,
      locale: parsed.locale === "uk" ? "uk" : "en",
    };
  } catch {
    return { soundsEnabled: true, musicEnabled: true, showMoveHints: true, locale: "en" };
  }
}

function saveWebSettings(settings: WebSettings): void {
  try {
    window.localStorage.setItem(WEB_SETTINGS_KEY, JSON.stringify(settings));
  } catch {
    // Settings are a nicety on Web; gameplay should never depend on localStorage.
  }
}

function soundForMoveResult(game: GameDto, targetPiece: BoardPiece | null): WebSoundId {
  if (game.status === "finished") {
    return "checkmate";
  }
  return targetPiece ? "capture" : "move";
}

function parseCurrentTarget(fen: string, square: string): BoardPiece | null {
  return parseFenBoard(fen)[squareToBoardIndex(square)] ?? null;
}

function squareToBoardIndex(square: string): number {
  const file = square.charCodeAt(0) - "a".charCodeAt(0);
  const rank = Number(square[1]);
  return (8 - rank) * 8 + file;
}

function playWebSound(id: WebSoundId, enabled: boolean): void {
  if (!enabled) {
    return;
  }

  const spec = webSoundSpec(id);
  const AudioContextCtor = window.AudioContext ?? (window as unknown as { webkitAudioContext?: typeof AudioContext }).webkitAudioContext;
  if (!AudioContextCtor || !spec) {
    return;
  }

  const context = new AudioContextCtor();
  const oscillator = context.createOscillator();
  const gain = context.createGain();
  const now = context.currentTime;
  const duration = spec.durationMs / 1000;

  oscillator.type = "sine";
  oscillator.frequency.setValueAtTime(spec.startHz, now);
  oscillator.frequency.exponentialRampToValueAtTime(spec.endHz, now + duration);
  gain.gain.setValueAtTime(0.0001, now);
  gain.gain.exponentialRampToValueAtTime(spec.gain, now + Math.min(0.015, duration * 0.25));
  gain.gain.exponentialRampToValueAtTime(0.0001, now + duration);

  oscillator.connect(gain);
  gain.connect(context.destination);
  oscillator.start(now);
  oscillator.stop(now + duration);
  window.setTimeout(() => void context.close(), spec.durationMs + 40);
}

function webSoundSpec(id: WebSoundId): { durationMs: number; startHz: number; endHz: number; gain: number } | null {
  switch (id) {
    case "move":
      return { durationMs: 56, startHz: 560, endHz: 420, gain: 0.08 };
    case "capture":
      return { durationMs: 92, startHz: 360, endHz: 220, gain: 0.1 };
    case "check":
      return { durationMs: 120, startHz: 780, endHz: 980, gain: 0.08 };
    case "checkmate":
      return { durationMs: 240, startHz: 520, endHz: 880, gain: 0.09 };
    case "illegal":
      return { durationMs: 96, startHz: 240, endHz: 160, gain: 0.07 };
    case "menu_select":
      return { durationMs: 44, startHz: 680, endHz: 580, gain: 0.055 };
    case "menu_back":
      return { durationMs: 48, startHz: 420, endHz: 320, gain: 0.05 };
    case "game_created":
      return { durationMs: 160, startHz: 520, endHz: 760, gain: 0.08 };
    case "game_joined":
      return { durationMs: 150, startHz: 600, endHz: 820, gain: 0.08 };
  }
}

function playWebMusic(): void {
  if (webMusicPlaying) {
    return;
  }

  const AudioContextCtor = window.AudioContext ?? (window as unknown as { webkitAudioContext?: typeof AudioContext }).webkitAudioContext;
  if (!AudioContextCtor) {
    return;
  }

  webMusicContext = new AudioContextCtor();
  webMusicPlaying = true;
  scheduleWebMusicLoop();
  webMusicTimer = window.setInterval(scheduleWebMusicLoop, 24000);
}

function stopWebMusic(): void {
  webMusicPlaying = false;
  if (webMusicTimer !== null) {
    window.clearInterval(webMusicTimer);
    webMusicTimer = null;
  }
  if (webMusicContext) {
    void webMusicContext.close();
    webMusicContext = null;
  }
}

function scheduleWebMusicLoop(): void {
  if (!webMusicContext || !webMusicPlaying) {
    return;
  }

  const notes = [261.63, 293.66, 329.63, 392.0, 440.0, 392.0, 329.63, 293.66];
  const start = webMusicContext.currentTime + 0.02;

  notes.forEach((frequency, index) => {
    scheduleWebMusicNote(webMusicContext as AudioContext, start + index * 3, frequency, 2.85);
  });
}

function scheduleWebMusicNote(context: AudioContext, start: number, frequency: number, duration: number): void {
  const oscillator = context.createOscillator();
  const gain = context.createGain();
  oscillator.type = "sine";
  oscillator.frequency.setValueAtTime(frequency, start);
  gain.gain.setValueAtTime(0.0001, start);
  gain.gain.exponentialRampToValueAtTime(0.025, start + 0.35);
  gain.gain.exponentialRampToValueAtTime(0.0001, start + duration);
  oscillator.connect(gain);
  gain.connect(context.destination);
  oscillator.start(start);
  oscillator.stop(start + duration);
}
