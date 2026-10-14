// Note: The whole thing is hard-coded that means it might contain bugs or things like that.
// TODO: Implement a builder for this
// TODO: Assertion checks...

let RuntimeContext = {};

const EXPORTED_METHOD_NAMES = [
    "Init",
    "Close",
    "Callback",
];

function CreateRuntimeContext(instance) {
    const _memory = instance.exports.memory;
    if (!_memory) { throw Error(`WebAssembly instance does export 'memory' field aka (WebAssembly.Memory)`); }

    function __helperCreateMemoryViews(mem) {
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

            HEAP     : new DataView(mem.buffer),
        };
    }

    const _exports = {};
    for (let i = 0; i < EXPORTED_METHOD_NAMES.length; ++i) {
        const _exportName = EXPORTED_METHOD_NAMES[i]
        const _export = instance.exports[_exportName];
        if (!_export || typeof(_export) !== "function") {
            throw Error(`WebAssembly instance does not export method '${_exportName}'`);
        }
        _exports[_exportName] = _export;
    }

    const _textDecoder = new TextDecoder("utf8");
    const _ctx = {
        memory: {
            wasmMemory : _memory,
            views      : __helperCreateMemoryViews(_memory),
            ensureViews() {
                if (!_ctx.memory.views.HEAP.buffer) {
                    _ctx.memory.views = __helperCreateMemoryViews(_ctx.memory.wasmMemory)
                }
            },
            isValidPointer(ptr) {
                return !(ptr < 0 || ptr >= _ctx.memory.wasmMemory.buffer.byteLength || !Number.isInteger(ptr));
            },
        },
        stringUtility: {
            UTF8_toString(ptr) {
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

function CreateLogModule() {
    const Log_Line = (logLevel, message) => {
        const _label = logLevel == 1 ? "Warn:" : logLevel == 2 ? "Error:" : "Log:";

        const _messageString = RuntimeContext.stringUtility.UTF8_toString(message);
        console.log(_label, _messageString);
    }
    return {
        Log_Line
    }
}

function CreateWindowModule(canvas) {
    let _context;
    let _pixel_buffer;
    let _width;
    let _height;

    const Init_Window = (width, height, pixel_buffer, title) => {
        if (!Number.isInteger(width) || width <= 0 ||
            !Number.isInteger(height) || height <= 0) {
            throw new RangeError("Invalid window dimensions");
        }

        canvas.width = width;
        canvas.height = height;

        canvas.style.width = `${width}px`;
        canvas.style.height = `${height}px`;

        _context = canvas.getContext("2d");
        if (!_context) {
            throw new Error("Could not create 2D canvas context");
        }

        _width = width;
        _height = height;
        _pixel_buffer = pixel_buffer;

        document.title =
            RuntimeContext.stringUtility.UTF8_toString(title);
    };

    const Close_Window = () => {};

    const Window_Should_Close = () => false;

    const Put_Pixel_Buffer = () => {
        if (!_context || _pixel_buffer == null) {
            throw new Error("Window has not been initialized");
        }

        // Refresh views if WebAssembly memory may have grown.
        RuntimeContext.memory.ensureViews();

        const heap = RuntimeContext.memory.views.HEAP_U8;
        const byteLength = _width * _height * 4;
        const byteOffset = _pixel_buffer;

        if (!RuntimeContext.memory.isValidPointer(_pixel_buffer)) {
            throw new RangeError("Pixel buffer is outside WebAssembly memory");
        }

        // WebAssembly pointer and subarray offsets are byte offsets.
        const pixels = new Uint8ClampedArray(
            heap.buffer,
            heap.byteOffset + byteOffset,
            byteLength
        );

        const imageData = new ImageData(pixels, _width, _height);
        _context.putImageData(imageData, 0, 0);
    };

    return {
        Init_Window,
        Close_Window,
        Window_Should_Close,
        Put_Pixel_Buffer,
    };
}

async function InitWasm(wasmFilepath, canvas) {
    const _wasmSource = await WebAssembly.instantiateStreaming(await fetch(wasmFilepath), {
        "Log" : CreateLogModule(),
        "Window" : CreateWindowModule(canvas),
    });

    RuntimeContext = CreateRuntimeContext(_wasmSource.instance);
}

export async function InitApplication(wasmFilepath, canvas) {
    await InitWasm(wasmFilepath, canvas);

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
