import { html, render, createElement, For, createSignal, children, createEffect, createMemo } from "./renderer.mjs";

function Button(props) {
  const label = props.children.toString();
  console.log("button label is", label);
  const node = createAnyNode("Button", { label });
  node.setProperty("onClick", props.onClick);
  return node;
}

function Text(props) {
  const text = children(() => props.children);
  const node = createLabelNode(String(text()));

  createEffect(() => node.setText(String(text())));

  return node;
}

function App() {
  const [count, setCount] = createSignal(1);
  const nums = createMemo(() => {
    const len = count();
    return new Array(len).fill(0).map((_, i) => i);
  });
  return html`<Column>
    <${Button} onClick=${() => setCount(count() + 1)}>Click me!<//>
    <${Text}>${() => count()}<//>
    <${For} each=${nums()}>${(num) => html`${num}`}<//>
  <//>`;
}

const root = getScene();
render(() => App(), root);
// console.log(JSON.stringify(root, (k, v) => (k === "parent" ? undefined : v)));
// const scene = getScene();
// const label = createLabelNode("Hello! from js");
// label.setPosition(100, 100);
// scene.addChild(label);