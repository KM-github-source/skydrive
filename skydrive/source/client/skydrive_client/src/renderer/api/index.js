import requests from "./require";

// 测试
export default {

    reqUserLogin: (params) => requests({ url: "/user/login", method: 'POST', data: params }),
    reqUserRegister: (params) => requests({ url: "/user/register", method: 'POST', data: params }),


    reqUploadFileNormal: (params) => requests({
        url: "/upload/normal",
        method: 'POST',
        data: params,
        headers: {
            "Content-Type": "multipart/form-data",
        },
    }),
    reqUploadMkdir: (params) => requests({ url: "/upload/mkdir", method: 'POST', data: params }),
    reqUploadCheckFileExist: (params) => requests({ url: "/upload/check", method: 'POST', data: params }),
    reqUploadFileFast: (params) => requests({ url: "/upload/fast", method: 'POST', data: params }),
    reqUploadFileFirst: (params) => requests({ url: "/upload/file-first", method: 'POST', data: params }),
    reqUploadDirFirst: (params) => requests({ url: "/upload/dir-first", method: 'POST', data: params }),
    reqUploadDirOver: (params) => requests({ url: "/upload/dir-over", method: 'POST', data: params }),


    reqChangeName: (params) => requests({ url: "/operate/change-name", method: 'POST', data: params }),
    reqCopyFile: (params) => requests({ url: "/operate/copy-file", method: 'POST', data: params }),
    reqCopyDir: (params) => requests({ url: "/operate/copy-dir", method: 'POST', data: params }),
    reqMoveFile: (params) => requests({ url: "/operate/move-file", method: 'POST', data: params }),
    reqDelFile: (params) => requests({ url: "/operate/del-file", method: 'POST', data: params }),


    reqDownloadFileNormal: (params) => requests({ url: "/download/file-normal", method: 'POST', data: params, responseType: 'arraybuffer' }),
    reqDownloadFileFirst: (params) => requests({ url: "/download/file-first", method: 'POST', data: params }),
    reqDownloadDirFirst: (params) => requests({ url: "/download/dir-first", method: 'POST', data: params }),
    reqDownloadFileOver: (params) => requests({ url: "/download/file-over", method: 'POST', data: params }),
    reqDownloadDirOver: (params) => requests({ url: "/download/dir-over", method: 'POST', data: params }),


    reqAllFileOfDir: (params) => requests({ url: "/operate/all-file", method: 'POST', data: params }),
    reqDownloadingFileList: (params) => requests({ url: "/getinfo/downloading-file-list", method: 'POST', data: params }),
    reqDownloadingDirList: (params) => requests({ url: "/getinfo/downloading-dir-list", method: 'POST', data: params }),
    reqUploadingFileList: (params) => requests({ url: "/getinfo/uploading-file-list", method: 'POST', data: params }),
    reqUploadingDirList: (params) => requests({ url: "/getinfo/uploading-dir-list", method: 'POST', data: params }),
    reqGetFileLogicId: (params) => requests({ url: "/getinfo/single-file", method: 'POST', data: params })
}