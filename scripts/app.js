import { html, render, createElement, For } from "./renderer.mjs";

function App() {
  const nums = [5, 6, 7];
  return html`
    <${For} each=${nums}>${(num) => html`<div>${num}</div>`}<//>
  `;
}

const root = createElement("root");
render(() => App(), root);
console.log("Hii");
console.log(JSON.stringify(root, (k, v) => (k === "parent" ? undefined : v)));