import "./style.css";

const app = document.querySelector<HTMLDivElement>("#app");

if (!app) {
  throw new Error("App root not found");
}

app.innerHTML = `
  <section class="shell">
    <h1>Cat Chess</h1>
    <p>No accounts. No timers. Just chess.</p>
    <div class="actions">
      <button type="button">Create game</button>
      <button type="button">Join game</button>
    </div>
  </section>
`;
