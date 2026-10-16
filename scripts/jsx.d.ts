import type { JSX as SolidJSX } from "solid-js";

declare global {
  interface Node {
    // type-only brand; never set at runtime
    readonly __node__: unique symbol;
  }
  namespace JSX {
    type Element = SolidJSX.Element;
    interface ElementChildrenAttribute { children: {}; }
    interface IntrinsicElements {}
  }
}

export {};