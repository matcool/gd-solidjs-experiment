import { createRenderer } from "solid-js/universal";
import htm from 'htm/mini';

export const {
  render, effect, memo, createComponent, createElement, createTextNode,
  insertNode, insert, spread, setProp, mergeProps, use,
} = createRenderer({
  createElement(type: string) {
    // console.log("RENDERER createElement", type);
    return createAnyNode(type, {})!;
  },
  createTextNode(text: string) {
    // console.log("RENDERER createTextNode", text);
    return createLabelNode(String(text));
  },
  isTextNode(node) {
    // console.log("RENDERER isTextNode", node?.debug);
    return node instanceof Label;
  },
  replaceText(node, text) {
    // console.log("RENDERER replaceText", node?.debug, text);
    (node as Label).setText(text);
  },
  insertNode(parent, node, anchor) {
    // console.log("RENDERER insertNode", parent?.debug, node?.debug, anchor?.debug);
    if (!anchor) {
      parent!.addChild(node!);
    } else {
      parent!.insertAfter(node!, anchor);
    }
    parent!.updateLayout();
  },
  removeNode(parent, node) {
    // console.log("RENDERER removeNode", parent?.debug, node?.debug);
    parent!.removeChild(node!);
    parent!.updateLayout();
  },
  setProperty(node, name, value) {
    // console.log("RENDERER setProperty", node?.debug, name?.debug, value?.debug);
    node!.setProperty(name, value);
  },
  getParentNode(node) {
    // console.log("RENDERER getParentNode", node?.debug);
    return node!.getParent() ?? undefined;
  },
  getFirstChild(node) {
    // console.log("RENDERER getFirstChild", node?.debug);
    return node!.getChildren()[0];
  },
  getNextSibling(node) {
    // console.log("RENDERER getNextSibling", node?.debug);
    const p = node!.getParent();
    if (!p) return undefined;
    const children = p.getChildren();
    const idx = children.findIndex(child => child.addrId === node!.addrId);
    if (idx === -1) return undefined;
    return children[idx + 1];
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

export function h(type: any, props: Record<string, any>, ...children: any[]) {
  if (typeof type === 'function') {
    const p: Record<string, any> = {};
    for (const [k, v] of Object.entries(props ?? {})) {
      if (typeof v === "function" && !k.startsWith("on")) {
        Object.defineProperty(p, k, { get: v, enumerable: true });
      } else {
        p[k] = v;
      }
    }
    p.children = children.length === 1 ? children[0] : children;
    return createComponent(type, p);
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

export type Component<P extends Record<string, any> = {}> = (props: P) => JSX.Element;
export type VoidComponent<P extends Record<string, any> = {}> = Component<P & { children?: never }>;
export type ParentComponent<P extends Record<string, any> = {}> = Component<P & { children?: any }>;