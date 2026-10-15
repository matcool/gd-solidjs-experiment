import { createRenderer } from "solid-js/universal";
import htm from 'htm';

export const {
  render, effect, memo, createComponent, createElement, createTextNode,
  insertNode, insert, spread, setProp, mergeProps, use,
} = createRenderer({
  createElement(type) {
    return createAnyNode(type, {});
  },
  createTextNode(text) {
    return createLabelNode(String(text));
  },
  isTextNode(node) {
    return node instanceof Label;
  },
  replaceText(node, text) {
    console.log("Replacing text! to", text);
    node.setText(text);
  },
  insertNode(parent, node, anchor) {
    if (!anchor) {
      parent.addChild(node);
    } else {
      parent.insertAfter(node, anchor);
    }
    parent.updateLayout();
  },
  removeNode(parent, node) {
    parent.removeChild(node);
    parent.updateLayout();
  },
  setProperty(node, name, value) {
    console.log("setting property", name, "to", value);
    node.setProperty(name, value);
  },
  getParentNode(node) {
    return node.getParent() ?? undefined;
  },
  getFirstChild(node) {
    return node.getChildren()[0];
  },
  getNextSibling(node) {
    const p = node.getParent();
    if (!p) return undefined;
    const children = p.getChildren();
    return children[children.indexOf(node) + 1];
  },
});

export {
  For,
  Show,
  Suspense,
  SuspenseList,
  Switch,
  Match,
  Index,
  ErrorBoundary,
  createSignal,
  createEffect,
  children,
  createMemo,
} from "solid-js";

function h(type, props, ...children) {
  if (typeof type === 'function') {
    const c = children.length === 1 ? children[0] : children;
    return createComponent(type, { ...props, children: c });
  }
  
  const node = createElement(type);
  for (const key in props ?? {}) {
    setProp(node, key, props[key]);
  }
  for (const child of children.flat()) {
    if (child === null || child === false) continue;
    if (typeof child === "function") {
      insert(node, child);
      continue;
    }
    const ch = typeof child === "object" ? child : createTextNode(String(child));
    insertNode(node, ch);
  }
  return node;
}

export const html = htm.bind(h);