export default {
    bufferToBlob: (buf) => new Blob([buf.buffer]),
    arrayBufferToBuffer: (ab) => {
        var buf = new Buffer(ab.byteLength);
        var view = new Uint8Array(ab);
        for (var i = 0; i < buf.length; ++i) {
            buf[i] = view[i];
        }
        return buf;
    },
}