declare const console: {
    log(...args: unknown[]): void;
    error(...args: unknown[]): void;
}

declare class Node {
    setPosition(x: number, y: number): void;

    addChild(child: Node): void;
    insertAfter(child: Node, after: Node): void;

    getParent(): Node | null;
    getChildren(): Node[];

    removeChild(child: Node): void;

    updateLayout(): void;

    setProperty(prop: string, value: any): void;
}

declare class Label extends Node {
    setText(text: string): void;
}

declare function getScene(): Node;

declare function createLabelNode(text: string): Label;

declare function createAnyNode(name: string, props: Record<string, any>): Node | null;