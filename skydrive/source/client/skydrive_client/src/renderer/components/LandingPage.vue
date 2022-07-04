<template>
  <div id="wrapper">
    <img id="logo" src="~@/assets/logo.png" alt="electron-vue" />
    <main>
      <div class="left-side">
        <span class="title"> Welcome to your new project! </span>
        <system-information></system-information>
      </div>

      <div class="right-side">
        <div class="doc">
          <div class="title">Getting Started</div>
          <p>
            electron-vue comes packed with detailed documentation that covers
            everything from internal configurations, using the project
            structure, building your application, and so much more.
          </p>
          <button
            @click="
              open('https://simulatedgreg.gitbooks.io/electron-vue/content/')
            "
          >
            Read the Docs</button
          ><br /><br />
        </div>
        <div class="doc">
          <div class="title alt">Other Documentation</div>
          <button class="alt" @click="open">Electron</button>
          <button class="alt" @click="downloadtest">下载文件</button>
          <button class="alt" @click="onClickUploadFile">点击上传文件</button>
          <button class="alt" @click="downloadFile">点击下载文件</button>
          <button class="alt" @click="test">点击测试计时器</button>
          <button class="alt" @click="changepage">点击切换界面</button>
          <button class="alt" @click="uploadDir">点击上传文件夹</button>
        </div>
      </div>
    </main>
  </div>
</template>

<script>
import SystemInformation from "./LandingPage/SystemInformation";

const fs = require("fs");
const path = require("path");

export default {
  name: "landing-page",
  components: { SystemInformation },
  data() {
    return {
      u_id: 1,
      fileList: [],
      uploadFilePath:
        "D:\\Mydata\\homework\\大三下\\课程\\计算机网络\\小组作业\\网盘\\test\\123.txt",
      downloadFileInfo: {
        downloading_id: "",
        file_len: "",
        slice_len: "",
        stop_flag: "",
      },
    };
  },
  methods: {
    open() {
      let lee = '李,"a  wait';
      console.log(lee);
      this.$api
        .reqUserLogin({
          user_name: lee,
          passwd: "123df,'\r\n\"",
        })
        .then((res) => {
          console.log("reqUserLogin", res);
        });
    },
    downloadtest() {
      axios({
        url: "https://dummyimage.com/400x400",
        method: "get",
        responseType: "arraybuffer",
        // data: {
        //   user_name: "12",
        //   passwd: "34",
        // },
      }).then((data) => {
        console.log(data);
        let Buf = toBuffer(data.data);
        console.log(Buf.length);

        let fd = fs.openSync("E:/test626.png", "w");

        fs.writeSync(fd, Buf, 0, 1096, null);
        fs.closeSync(fd);
        function toBuffer(ab) {
          var buf = new Buffer(ab.byteLength);
          var view = new Uint8Array(ab);
          for (var i = 0; i < buf.length; ++i) {
            buf[i] = view[i];
          }
          return buf;
        }
      });
    },
    async downloadFileFirst(file_name) {
      let _this = this;
      await this.$api
        .reqDownloadFileFirst({
          u_id: "1",
          logic_id: "48",
          // file_real_path: "E:/mytest.txt"
          file_real_path: file_name,
        })
        .then((data) => {
          if (data.msg == "success") {
            _this.downloadFileInfo.downloading_id = data.downloading_id;
            _this.downloadFileInfo.file_len = data.file_len;
            _this.downloadFileInfo.slice_len = data.slice_len;
            _this.downloadFileInfo.stop_flag = 0;

            console.log("download first success!");
          } else {
            console.log("download first error!");
          }
        });
    },
    async downloadFileNormal(fd, start_p) {
      var fs = require("fs");
      let _this = this;
      for (
        let fpointer = start_p;
        fpointer < _this.downloadFileInfo.file_len;

      ) {
        if (_this.downloadFileInfo.stop_flag == 1) break;
        await this.$api
          .reqDownloadFileNormal({
            downloading_id: _this.downloadFileInfo.downloading_id,
            file_pointer: fpointer,
          })
          .then((data) => {
            console.log(data);
            let Buf = toBuffer(data);
            console.log(Buf);
            // let fd = fs.openSync(file_name, "w");
            fs.writeSync(fd, Buf, fpointer, Buf.length, null);
            // fs.closeSync(fd);
            function toBuffer(ab) {
              var buf = new Buffer(ab.byteLength);
              var view = new Uint8Array(ab);
              for (var i = 0; i < buf.length; ++i) {
                buf[i] = view[i];
              }
              return buf;
            }
            // console.log(_this.downloadFileInfo.file_len);
            // console.log(Buf.length);
            fpointer += Buf.length;
            // console.log(fpointer);
          });
        // break;
      }
    },
    async downloadFile() {
      console.log("i am in");
      let file_name = "E:/mytest.txt";
      await this.downloadFileFirst(file_name);
      let fd = fs.openSync(file_name, "w");
      const stat = fs.statSync(file_name);
      console.log(stat.size);
      await this.downloadFileNormal(fd, stat.size);
      fs.closeSync(fd);
    },
    async uploadFile(filepath, parent_id, uploading_dir_id) {
      // 取文件信息，包括md5、长度
      let file_name = this.$common.getFileNameByPath(filepath);
      let file_stats;
      let res;
      try {
        file_stats = fs.statSync(filepath);
      } catch (error) {
        console.error("fs", error);
        return;
      }
      let file_len = file_stats.size;
      let md5 = this.$common.getFileMd5(filepath);
      console.log("file_len", file_len);

      // 首先，查看是否可以秒传
      res = await this.$api.reqUploadCheckFileExist({ md5 });
      if (res.is_exist == 1) {
        res = await this.$api.reqUploadFileFast({
          parent_id,
          file_name,
          md5,
        });
        if (res.msg === "success") console.log(file_name, "秒传成功");
        else console.log(file_name, "秒传失败", res.msg);
        return;
      }
      // 发送first请求
      res = await this.$api.reqUploadFileFirst({
        u_id: this.u_id,
        file_real_path: filepath,
        md5,
        file_len,
        parent_id,
        file_name,
        uploading_dir_id,
      });
      console.log("reqUploadFileFirst", res);
      if (res.msg !== "success") {
        console.log(res.msg);
        return;
      }
      let file_pointer = Number(res.file_pointer);
      let uploading_id = Number(res.uploading_id);
      let slice_len = (this.slice_len = Number(res.slice_len));

      // 循环读取文件，进行发送
      let file_content = new Buffer(slice_len);
      let fd = fs.openSync(filepath, "r");
      while (file_pointer < file_len - 1) {
        let uploading_len = fs.readSync(
          fd,
          file_content,
          0,
          slice_len,
          file_pointer
        );
        console.log("file_pointer", file_pointer);
        console.log("uploading_len", uploading_len);
        let form_data = new FormData();
        form_data.append("uploading_id", uploading_id);
        form_data.append("uploading_len", uploading_len);
        form_data.append(
          "file_content",
          this.$common.bufferToBlob(file_content)
        );
        res = await this.$api.reqUploadFileNormal(form_data);
        if (res.msg !== "success") {
          console.log("reqUploadFileNormal", res.msg);
          continue;
        }
        file_pointer = Number(res.file_pointer);
        // 更新进度条
      }
      fs.close(fd);
    },
    myGetfileCount(MyUrl) {
      let mycnt = 0;
      let files = fs.readdirSync(MyUrl);
      files.forEach((file) => {
        let fPath = path.join(MyUrl, file);
        let stat = fs.statSync(fPath);
        if (stat.isDirectory()) {
          let ret = this.myGetfileCount(fPath);
          mycnt += ret;
        } else {
          mycnt++;
          console.log(file);
        }
      });
      return mycnt;
    },
    async uploadDirRecursion(MyUrl, parent_id, cnt, cntnow, uploading_dir_id) {
      let mycnt = cnt;
      let files = fs.readdirSync(MyUrl);
      console.log(files);

      for (let i = 0; i < files.length; i++) {
        let file = files[i];
        let fPath = path.join(MyUrl, file);
        let goodPath = this.$common.replaceEscape(fPath);
        let stat = fs.statSync(fPath);
        if (stat.isDirectory()) {
          let ret0 = await this.$api.reqUploadMkdir({
            file_name: file,
            parent_id: parent_id,
          });
          let ret = await this.uploadDirRecursion(
            fPath,
            ret0.id,
            mycnt,
            cntnow,
            uploading_dir_id
          );
          mycnt = ret;
        } else {
          console.log(file);
          mycnt++;
          if (mycnt > cntnow) {
            console.log(mycnt, file, goodPath, parent_id, uploading_dir_id);
            await this.uploadFile(goodPath, parent_id, uploading_dir_id);
          }
        }
      }
      return mycnt;
    },
    async uploadDir() {
      let dir_path = "D:/Mydata/homework/大三下/课程/计算机网络/小作业/3.2";
      let file_count = this.myGetfileCount(dir_path);
      console.log("数量", file_count);
      let res;
      let firstpack = {
        u_id: 3,
        file_real_path: dir_path,
        parent_id: 188,
        dir_name: "3.2",
        file_count: file_count,
      };
      res = await this.$api.reqUploadDirFirst(firstpack);
      console.log("dirfirst", res);
      let uploading_id = res.uploading_id;
      res = await this.uploadDirRecursion(
        dir_path,
        res.logic_id,
        0,
        Number(res.file_now),
        res.uploading_id
      );
      res = await this.$api.reqUploadDirOver({
        uploading_id: uploading_id,
      });
    },
    async onClickUploadFile() {
      // // 取文件名
      // let file_name = this.$common.getFileNameByPath(this.uploadFilePath);
      // if (file_name === "") {
      //   console.log("选择路径出错");
      //   return;
      // }

      // // 查找云端是否有同名，有则提示是否覆盖
      // let have_same_file = false;
      // this.fileList.forEach((f) => {
      //   if (f.file_name === file_name) have_same_file = true;
      // });
      // if (have_same_file) {
      //   // 弹窗提示
      // }

      this.uploadFile(
        "D:/Mydata/homework/大三下/课程/计算机网络/小组作业/网盘/test/计网秒传测试.rar",
        -1,
        -1
      );
    },
    downloadFile() {
      // axios({
      //   url: "http://139.9.243.37:40000/download/file-normal",
      //   method: "POST",
      //   data: {
      //     downloading_id: 23,
      //     file_pointer: 12
      //   },
      //   withCredentials: true,
      // })
      axios
        .post("http://139.9.243.37:40000/download/file-normal", {
          downloading_id: 23,
          file_pointer: 12,
        })
        .then((data) => {
          console.log(data);
          // let Buf = toBuffer(data.data);
          // console.log(Buf);

          // let fd = fs.openSync("E:/test626.png", "w");

          // fs.writeSync(fd, Buf, 0, 1096, null);
          // fs.closeSync(fd);
          // function toBuffer(ab) {
          //   var buf = new Buffer(ab.byteLength);
          //   var view = new Uint8Array(ab);
          //   for (var i = 0; i < buf.length; ++i) {
          //     buf[i] = view[i];
          //   }
          //   return buf;
          // }
        })
        .catch((err) => {
          console.log("err", err);
        });
    },
    test() {
      setInterval(() => {
        console.log("123");
      }, 1000);
    },
    changepage() {
      this.$router.push({
        name: "login",
      });
    },
  },
};
</script>

<style>
@import url("https://fonts.googleapis.com/css?family=Source+Sans+Pro");

* {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

body {
  font-family: "Source Sans Pro", sans-serif;
}

#wrapper {
  background: radial-gradient(
    ellipse at top left,
    rgba(255, 255, 255, 1) 40%,
    rgba(229, 229, 229, 0.9) 100%
  );
  height: 100vh;
  padding: 60px 80px;
  width: 100vw;
}

#logo {
  height: auto;
  margin-bottom: 20px;
  width: 420px;
}

main {
  display: flex;
  justify-content: space-between;
}

main > div {
  flex-basis: 50%;
}

.left-side {
  display: flex;
  flex-direction: column;
}

.welcome {
  color: #555;
  font-size: 23px;
  margin-bottom: 10px;
}

.title {
  color: #2c3e50;
  font-size: 20px;
  font-weight: bold;
  margin-bottom: 6px;
}

.title.alt {
  font-size: 18px;
  margin-bottom: 10px;
}

.doc p {
  color: black;
  margin-bottom: 10px;
}

.doc button {
  font-size: 0.8em;
  cursor: pointer;
  outline: none;
  padding: 0.75em 2em;
  border-radius: 2em;
  display: inline-block;
  color: #fff;
  background-color: #4fc08d;
  transition: all 0.15s ease;
  box-sizing: border-box;
  border: 1px solid #4fc08d;
}

.doc button.alt {
  color: #42b983;
  background-color: transparent;
}
</style>