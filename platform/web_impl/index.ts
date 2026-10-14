// TODO: This is first draft, so it may contain bugs related to memory.
//       I have yet to discover the bugs.
//
// TODO: A lot of things are hard-coded or assumed to not fail.
//       Implement something like 'assert' and remove dependence on hard-coded behavior.

interface RuntimeContext_T {
    memory: {
        wasmMemory: WebAssembly.Memory

        views: {
            HEAP_U8  : Uint8Array
            HEAP_U16 : Uint16Array
            HEAP_U32 : Uint32Array
            HEAP_U64 : BigUint64Array

            HEAP_I8  : Int8Array
            HEAP_I16 : Int16Array
            HEAP_I32 : Int32Array
            HEAP_I64 : BigInt64Array

            HEAP_F32 : Float32Array
            HEAP_F64 : Float64Array

            HEAP     : DataView<ArrayBufferLike>
        }

        ensureViews(): void
        isValidPointer(ptr: number): boolean
    }

    stringUtility: {
        UTF8ToString (ptr: number): string
    }

    exports: Record<string, Function>
};
let RuntimeContext = {} as RuntimeContext_T;

function createRuntimeContext(instance: WebAssembly.Instance): RuntimeContext_T {
    const _memory = instance.exports.memory! as WebAssembly.Memory;
    if (!_memory) { throw Error(`WebAssembly instance does export 'memory' field aka (WebAssembly.Memory)`); }

    function __helperCreateMemoryViews(mem: WebAssembly.Memory): RuntimeContext_T["memory"]["views"] {
        return {
            HEAP_U8  : new Uint8Array               (mem.buffer),
            HEAP_U16 : new Uint16Array              (mem.buffer),
            HEAP_U32 : new Uint32Array              (mem.buffer),
            HEAP_U64 : new BigUint64Array           (mem.buffer),

            HEAP_I8  : new Int8Array                (mem.buffer),
            HEAP_I16 : new Int16Array               (mem.buffer),
            HEAP_I32 : new Int32Array               (mem.buffer),
            HEAP_I64 : new BigInt64Array            (mem.buffer),

            HEAP_F32 : new Float32Array             (mem.buffer),
            HEAP_F64 : new Float64Array             (mem.buffer),

            HEAP     : new DataView<ArrayBufferLike>(mem.buffer),
        };
    }

    const EXPORTED_METHOD_NAMES: string[]= [ "Init", "Callback", "Close", ]

    const _exports: RuntimeContext_T["exports"] = {} as RuntimeContext_T["exports"];
    for (let i = 0; i < EXPORTED_METHOD_NAMES.length; ++i) {
        const _exportName = EXPORTED_METHOD_NAMES[i]
        const _export = instance.exports[_exportName]! as Function;
        if (!_export || typeof(_export) !== "function") {
            throw Error(`WebAssembly instance does not export method '${_exportName}'`);
        }
        _exports[_exportName] = _export!;
    }

    const _textDecoder = new TextDecoder("utf8");
    const _ctx: RuntimeContext_T = {
        memory: {
            wasmMemory : _memory,
            views      : __helperCreateMemoryViews(_memory),
            ensureViews() {
                if (!_ctx.memory.views.HEAP.buffer) {
                    _ctx.memory.views = __helperCreateMemoryViews(_ctx.memory.wasmMemory)
                }
            },
            isValidPointer(ptr: number): boolean {
                return !(ptr < 0 || ptr >= _ctx.memory.wasmMemory.buffer.byteLength || !Number.isInteger(ptr));
            },
        },
        stringUtility: {
            UTF8ToString(ptr: number): string {
                if (!_ctx.memory.isValidPointer(ptr)) { throw Error(`Invalid pointer ${ptr}`); }
                _ctx.memory.ensureViews();
                const _u8Buf = _ctx.memory.views.HEAP_U8;
                let _len = 0;
                while (_len + ptr < _u8Buf.byteLength && _u8Buf[_len + ptr]) { _len++; }
                return _textDecoder.decode(_u8Buf.subarray(ptr, ptr + _len));
            },
        },
        exports: _exports
    };
    return _ctx;
}

function createLogModule() {
    const Log = (message: number) => {
        console.log(RuntimeContext.stringUtility.UTF8ToString(message));
    };

    return { Log };
}

function createWindowModule(canvas: HTMLCanvasElement) {
    let _width: number;
    let _height: number;
    let _pixel_buffer: number;

    const _context2d = canvas.getContext("2d")!;

    const Init_Window = (width: number, height: number, pixel_buffer: number, title: number) => {
        _width = width;
        _height = height;
        _pixel_buffer = pixel_buffer;

        canvas.width = _width;
        canvas.height = _height;

        canvas.style.width = _width + "px";
        canvas.style.height = _height + "px";

        document.title = RuntimeContext.stringUtility.UTF8ToString(title);
    };

    const Close_Window = () => {
    };

    const Window_Should_Close = () => {
        return false;
    };

    const Put_Pixel_Buffer = () => {
        RuntimeContext.memory.ensureViews();

        const _heap = RuntimeContext.memory.views.HEAP_U8;
        const _byteLength = _width * _height * 4;
        const _byteOffset = _pixel_buffer;

        if (!RuntimeContext.memory.isValidPointer(_pixel_buffer)) {
            throw new RangeError("Pixel buffer is outside WebAssembly memory or is invalid pointer.");
        }

        const _pixels = new Uint8ClampedArray(
            _heap.buffer,
            _heap.byteOffset + _byteOffset,
            _byteLength
        );

        const _imageData = new ImageData(_pixels as ImageDataArray, _width, _height);
        _context2d.putImageData(_imageData, 0, 0);
    };

    return {
        Init_Window,
        Close_Window,
        Window_Should_Close,
        Put_Pixel_Buffer,
    };
}

async function initWasm(wasmFilepath: string, parentDivContainer: HTMLDivElement) {
    parentDivContainer.replaceChildren(); // empty it.
    const _canvas = document.createElement("canvas")!;
    parentDivContainer.appendChild(_canvas);

    const _wasmImports = {
        "Log": createLogModule(),
        "Window": createWindowModule(_canvas),
    } as Record<string, Record<string, Function>>;

    const _wasmSource = await WebAssembly.instantiateStreaming(await fetch(wasmFilepath), _wasmImports);
    RuntimeContext = createRuntimeContext(_wasmSource.instance);
}

export async function mpp_InitApplication(wasmFilepath: string, parentDivElement: HTMLDivElement) {
    await initWasm(wasmFilepath, parentDivElement);

    if (RuntimeContext.exports.Init() != 0) {
        console.log("Init function failed");
        return;
    }

    function frameCallback() {
        if (!RuntimeContext.exports.Callback()) {
            RuntimeContext.exports.Close();
            return;
        }

        requestAnimationFrame(frameCallback);
    }
    requestAnimationFrame(frameCallback);
}
