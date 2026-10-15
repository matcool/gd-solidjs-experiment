declare const console: {
    log(...args: unknown[]): void;
    error(...args: unknown[]): void;
}

declare class Node {
    addChild(child: Node): void;
}

declare function getScene(): Node;