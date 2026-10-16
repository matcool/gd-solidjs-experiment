import { render, For, createSignal, children, createEffect, createMemo, insert } from "./renderer";
import type { Component, ParentComponent } from "./renderer";

const Column: ParentComponent = (props) => {
  const kids = children(() => props.children);
  const node = createAnyNode("Column", {})!;
  insert(node, kids);
  return node;
};

const Button: ParentComponent<{onClick: () => void}> = (props) => {
  const label = props.children!.toString();
  const node = createAnyNode("Button", { label })!;
  node!.setProperty("onClick", props.onClick);
  return node;
};

const Text: ParentComponent = (props) => {
  const text = children(() => props.children);
  const node = createLabelNode(String(text()));

  createEffect(() => {
    // console.log("updating Text to", text());
    node.setText(String(text()))
  });

  return node;
};

type Foo = JSX.Element;

const App: Component = () => {
  const [count, setCount] = createSignal(1);
  const nums = createMemo(() => {
    const len = count();
    return new Array(len).fill(0).map((_, i) => i);
  });
  return <Column>
    <Button onClick={() => setCount(count() + 1)}>Click me!</Button>
    <Text>{count()}</Text>
    <For each={nums()}>{
      (num) => <Text>{num}</Text>
    }</For>
  </Column>;
}

const root = getScene();
render(() => App({}), root);