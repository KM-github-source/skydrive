const fs = require("fs");
const crypto = require("crypto");

export default {
    getFileMd5: (path) =>
        crypto
            .createHash("md5")
            .update(fs.readFileSync(path), "utf8")
            .digest("hex"),
    getFileNameByPath: (path) => {
        let index = path.lastIndexOf("/");
        if (index === -1)
            index = path.lastIndexOf("\\");
        return path.substr(index + 1);
    },
    bufferToBlob: (buf) => new Blob([buf.buffer]),
    arrayBufferToBuffer: (ab) => {
        var buf = new Buffer(ab.byteLength);
        var view = new Uint8Array(ab);
        for (var i = 0; i < buf.length; ++i) {
            buf[i] = view[i];
        }
        return buf;
    },
    replaceEscape: (s) => s.replace(/\\/g, "/"),
}