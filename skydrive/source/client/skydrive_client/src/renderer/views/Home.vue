/<template>
  <div class="container">
    <div class="mask" v-show="!!alert_content">
      <el-alert
        class="myalert"
        v-show="!!alert_content"
        :title="alert_content"
        type="error"
        show-icon
        @close="closeError"
      >
      </el-alert>
    </div>
    <div class="aside">
      <div
        class="tab-button"
        :class="{
          'hover-tab-button': index === 1,
        }"
        @click="changeMain(1)"
      >
        <i class="el-icon-tickets"></i>
        <span>文件</span>
      </div>
      <div
        class="tab-button"
        :class="{
          'hover-tab-button': index === 2,
        }"
        @click="changeMain(2)"
      >
        <i class="el-icon-upload2"></i>
        <span>上传列表</span>
      </div>
      <div
        class="tab-button"
        :class="{
          'hover-tab-button': index === 3,
        }"
        @click="changeMain(3)"
      >
        <i class="el-icon-download"></i>
        <span>下载列表</span>
      </div>
      <div class="tab-button" @click="logout">
        <i class="el-icon-circle-close"></i>
        <span>登出</span>
      </div>
    </div>
    <div class="main">
      <router-view></router-view>
    </div>
  </div>
</template>

<script>
import { mapState, mapMutations } from "vuex";
const fs = require("fs");
const path = require("path");

export default {
  data() {
    return {
      index: 1,
      alert_content: "",
      uploading_file_map: {},
      uploading_dir_map: {},
      downloading_file_map: {},
      downloading_dir_map: {},

      // downloadFileInfo: {
      //   downloading_id: "",
      //   file_len: "",
      //   slice_len: "",
      //   stop_flag: "",
      // },
      // downloadDirInfo: {
      //   // top_dir_id: '',
      //   downloading_id: "",
      //   file_now: "",
      //   file_count: "",
      //   stop_flag: "",
      // },
    };
  },
  computed: {
    ...mapState("Main", [
      "u_id",
      "root_id",
      "user_name",
      "copy_info",
      "move_info",
      // "uploading_list",
      // "downloading_list",
    ]),
  },
  async mounted() {
    this.$common.showError = this.showError.bind(this);
    this.$common.uploadFile = this.uploadFile.bind(this);
    this.$common.uploadDir = this.uploadDir.bind(this);

    this.$common.downloadFile = this.downloadFile.bind(this);
    this.$common.downloadDirTop = this.downloadDirTop.bind(this);

    this.$common.getMap = this.getMap.bind(this);
    this.$common.setMap = this.setMap.bind(this);
    this.$common.addMap = this.addMap.bind(this);
    this.$common.delMap = this.delMap.bind(this);

    this.uploading_file_map = await this.getUpFileList();
    this.uploading_dir_map = await this.getUpDirList();
    this.downloading_file_map = await this.getDownFileList();
    this.downloading_dir_map = await this.getDownDirList();
  },
  methods: {
    ...mapMutations("Main", [
      "changeUid",
      "changeRootid",
      "changeUsername",
      "changeCopyinfo",
      "changeMoveInfo",
      // "changeUploadingList",
      // "addUploadingItem",
      // "delUploadingItem",
      // "updateUploadingItem",
      // "changeUploadingDirList",
      // "addUploadingDirItem",
      // "delUploadingDirItem",
      // "updateUploadingDirItem",
      // "changeDownloadingList",
    ]),
    getMap(map_name) {
      return this[map_name];
    },
    setMap(map_name, id, key, value) {
      this.$set(this[map_name][id], key, value);
    },
    addMap(map_name, item) {
      if (!(item.id in this[map_name]))
        this.$set(this[map_name], item.id, item);
    },
    delMap(map_name, id) {
      if (id in this[map_name]) this.$delete(this[map_name], id);
    },
    changeMain(index) {
      if (index == this.index) return;
      this.index = index;
      let name;
      if (index === 1) name = "file";
      else if (index === 2) name = "uploading";
      else name = "downloading";
      this.$router.replace({
        name,
      });
    },
    showError(prompt) {
      this.alert_content = prompt;
    },
    closeError() {
      this.alert_content = "";
    },
    logout() {
      // 清空用户信息
      this.$confirm("确认登出？", "提示", {
        confirmButtonText: "确定",
        cancelButtonText: "取消",
        type: "warning",
      }).then(() => {
        this.changeUid("");
        this.$router.replace({ name: "login" });
      });
    },
    async getUpFileList() {
      // 取得所有上传信息表
      let uploading_file_list = await this.$api.reqUploadingFileList({
        u_id: this.u_id,
      });
      console.log("flist", uploading_file_list);
      let ret = {};
      uploading_file_list.forEach((element) => {
        if (element.uploading_dir_id != -1) return;
        element.cnt_now = element.file_pointer;
        element.cnt_target = element.file_len;
        element.is_dir = false;
        element.is_pause = true;
        ret[element.id] = element;
      });
      return ret;
    },
    async getUpDirList() {
      // 取得所有上传目录信息表
      let uploading_dir_list = await this.$api.reqUploadingDirList({
        u_id: this.u_id,
      });
      console.log("dirlist", uploading_dir_list);
      let ret = {};
      uploading_dir_list.forEach((element) => {
        element.cnt_now = element.file_now;
        element.cnt_target = element.file_count;
        element.is_dir = true;
        element.is_pause = true;
        ret[element.id] = element;
      });
      return ret;
    },
    async getDownFileList() {
      // 取得所有上传信息表
      let downloading_file_list = await this.$api.reqDownloadingFileList({
        u_id: this.u_id,
      });
      console.log("flist", downloading_file_list);
      let ret = {};
      downloading_file_list.forEach((element) => {
        // if (element.downloading_dir_id != -1) return;
        element.cnt_now = element.file_pointer;
        element.cnt_target = element.file_len;
        element.is_dir = false;
        element.is_pause = true;
        ret[element.id] = element;
      });
      console.log("flist ret:", ret);
      return ret;
    },
    async getDownDirList() {
      // 取得所有上传目录信息表
      let downloading_dir_list = await this.$api.reqDownloadingDirList({
        u_id: this.u_id,
      });
      console.log("dirlist", downloading_dir_list);
      let ret = {};
      downloading_dir_list.forEach((element) => {
        element.cnt_now = element.file_now;
        element.cnt_target = element.file_count;
        element.is_dir = true;
        element.is_pause = true;
        ret[element.id] = element;
      });
      return ret;
    },
    async uploadFile(filepath, parent_id, uploading_dir_id) {
      // 取文件信息，包括md5、长度
      filepath = this.$common.replaceEscape(filepath);
      let file_name = this.$common.getFileNameByPath(filepath);
      let file_stats;
      let res;
      try {
        file_stats = fs.statSync(filepath);
      } catch (error) {
        console.error("fs", error);
        return -1;
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
        if (res.msg === "success") 
        {
          console.log(file_name, "秒传成功"); 
          this.$message.success("秒传成功");
        }
        else 
          console.log(file_name, "秒传失败", res.msg);
        return 0;
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
        return -1;
      }
      let file_pointer = Number(res.file_pointer);
      let uploading_id = Number(res.uploading_id);
      let slice_len = (this.slice_len = Number(res.slice_len));
      // 更新前端待上传文件列表
      if (file_len && uploading_dir_id == -1 && file_pointer == 0) {
        let upl = await this.getUpFileList();
        for (let key in upl) {
          if (!(key in this.uploading_file_map)) {
            upl[key].is_pause = false;
            this.addMap("uploading_file_map", upl[key]);
            break;
          }
        }
      }

      // 循环读取文件，进行发送
      let file_content = new Buffer(slice_len);
      let fd = fs.openSync(filepath, "r");
      let tryagaincnt = 0;
      while (file_pointer < file_len - 1) {
        // 判断是否按了暂停
        if (
          uploading_dir_id == -1 &&
          this.uploading_file_map[uploading_id].is_pause
        ) {
          fs.close(fd);
          return -2;
        }
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
        form_data.append("file_pointer", file_pointer);
        form_data.append(
          "file_content",
          this.$common.bufferToBlob(file_content)
        );
        res = await this.$api.reqUploadFileNormal(form_data);
        if (res.msg !== "success") {
          console.log("reqUploadFileNormal", res.msg);
          if (tryagaincnt >= 1) 
          {
            fs.close(fd);
            // 更新暂停信息
            //console.log("邬嘉晟的测试pause");
            if (uploading_dir_id == -1) {
              this.setMap(
                "uploading_file_map",
                uploading_id,
                "is_pause",
                true
              );
            }
            return -1;
          } else {
            //console.log("邬嘉晟的测试tryagain");
            tryagaincnt++;
            continue;
          }
        }
        tryagaincnt = 0;
        file_pointer = Number(res.file_pointer);
        // 更新进度条
        if (uploading_dir_id == -1)
          this.setMap(
            "uploading_file_map",
            uploading_id,
            "cnt_now",
            file_pointer
          );
      }
      fs.close(fd);
      // 更新前端vuex待下载文件列表
      if (file_len && uploading_dir_id == -1)
        this.delMap("uploading_file_map", uploading_id);
      return 0;
    },
    async uploadDir(dir_path, dir_name, parent_id) {
      let file_count = this.myGetfileCount(dir_path);
      console.log("数量", file_count);
      let res;
      let firstpack = {
        u_id: this.u_id,
        file_real_path: this.$common.replaceEscape(dir_path),
        parent_id,
        dir_name,
        file_count,
      };
      res = await this.$api.reqUploadDirFirst(firstpack);
      if (res.msg != "success") return -1;
      console.log("dirfirst", res);
      let uploading_id = res.uploading_id;
      // 更新前端vuex待下载文件列表
      if (file_count && Number(res.file_now) == 0) {
        let upl = await this.getUpDirList();
        for (let key in upl) {
          if (!(key in this.uploading_dir_map)) {
            upl[key].is_pause = false;
            this.addMap("uploading_dir_map", upl[key]);
            break;
          }
        }
      }
      res = await this.uploadDirRecursion(
        dir_path,
        res.logic_id,
        0,
        Number(res.file_now),
        res.uploading_id
      );
      if (res == -1) 
      {
        console.log("dirmap:",this.uploading_dir_map);
        this.setMap(
          "uploading_dir_map",
          uploading_id,
          "is_pause",
          true
        );
          return -1;
      }
      res = await this.$api.reqUploadDirOver({
        uploading_id: uploading_id,
      });
      console.log("文件夹上传完毕，upl_id,res", uploading_id, res);
      if (res.msg != "success") return -1;
      // 更新前端vuex待下载文件列表
      if (file_count) this.delMap("uploading_dir_map", uploading_id);
      return 0;
    },
    async uploadDirRecursion(MyUrl, parent_id, cnt, cntnow, uploading_dir_id) {
      let mycnt = cnt;
      let files = fs.readdirSync(MyUrl);
      console.log(files);

      for (let i = 0; i < files.length; i++) {
        mycnt++;
        // 更新进度条
        this.setMap("uploading_dir_map", uploading_dir_id, "cnt_now", mycnt);
        let file = files[i];
        let fPath = path.join(MyUrl, file);
        let goodPath = this.$common.replaceEscape(fPath);
        let stat = fs.statSync(fPath);
        if (stat.isDirectory()) {
          let ret0;
          if (mycnt > cntnow) {
            ret0 = await this.$api.reqUploadMkdir({
              file_name: file,
              parent_id: parent_id,
              uploading_id: uploading_dir_id,
            });
          } else {
            ret0 = await this.$api.reqGetFileLogicId({
              file_name: file,
              parent_id: parent_id,
              is_dir: 1,
            });
            console.log("查询文件夹", ret0);
          }

          if (ret0.msg != "success") return -1;

          console.log("parent_id:", ret0.id);
          let ret = await this.uploadDirRecursion(
            fPath,
            ret0.id,
            mycnt,
            cntnow,
            uploading_dir_id
          );
          if (ret == -1) return -1;
          mycnt = ret;
        } else {
          console.log(file);
          if (mycnt > cntnow) {
            console.log(mycnt, file, goodPath, parent_id, uploading_dir_id);
            let ret1 = await this.uploadFile(
              goodPath,
              parent_id,
              uploading_dir_id
            );
            if (ret1 == -1) return -1;
          }
        }
      }
      return mycnt;
    },
    myGetfileCount(MyUrl) {
      let mycnt = 0;
      let files = fs.readdirSync(MyUrl);
      files.forEach((file) => {
        mycnt++;
        let fPath = path.join(MyUrl, file);
        let stat = fs.statSync(fPath);
        if (stat.isDirectory()) {
          let ret = this.myGetfileCount(fPath);
          mycnt += ret;
        } else {
          console.log(file);
        }
      });
      return mycnt;
    },

    // 下载
    async downloadFileFirst(file_id, file_name, downloading_dir_id) {
      let _this = this;
      console.log("dir id : ", downloading_dir_id);
      return await this.$api
        .reqDownloadFileFirst({
          u_id: _this.u_id,
          logic_id: file_id,
          // file_real_path: "E:/mytest.txt"
          file_real_path: file_name,
          downloading_dir_id: downloading_dir_id,
        })
        .then(async(data) => {
          if (data.msg == "success") {
            // 更新前端待下载文件列表
            // if (downloading_dir_id == -1) {
            if (data.downloading_id in this.downloading_file_map)
              this.downloading_file_map[data.downloading_id].is_pause = false;
            else {
              let upl = await this.getDownFileList();
              console.log("upl:", upl);
              console.log("downloading_file_map:", this.downloading_file_map);
              for (let key in upl) {
                console.log("key in upl:",key);
                if (!(key in this.downloading_file_map)) {
                  upl[key].is_pause = false;
                  this.addMap("downloading_file_map", upl[key]);
                  break;
                }
              }
            }
            // }
            // _this.downloadFileInfo.downloading_id = data.downloading_id;
            // _this.downloadFileInfo.file_len = data.file_len;
            // _this.downloadFileInfo.slice_len = data.slice_len;
            // _this.downloadFileInfo.stop_flag = 0;
            console.log("download first success!");
            console.log("id:", data.downloading_id);
            return data.downloading_id;
          } else {
            console.log("download first error!");
            return -1;
          }
        });
    },
    async downloadFileNormal(fd, start_p, downloading_id) {
      let _this = this;
      console.log("downId:", downloading_id);
      console.log("dfm:", this.downloading_file_map[downloading_id]);
      let try_chance = 3;
      for (
        let fpointer = start_p;
        fpointer < this.downloading_file_map[downloading_id].file_len;

      ) {
        if (this.downloading_file_map[downloading_id].is_pause) return -2;
        let data;
        let Buf;
        try {
          data = await this.$api.reqDownloadFileNormal({
            downloading_id: downloading_id,
            file_pointer: fpointer,
          });
          console.log("data:", data);
          Buf = this.$common.arrayBufferToBuffer(data);
          console.log(Buf);
          try_chance = 3;
        } catch (err) {
          console.log("data err! ", err);
          // fs.closeSync(fd);
          try_chance--;
          if(try_chance <= 0)
            return -1;
          continue;
        }
        fs.writeSync(fd, Buf, 0, Buf.length, fpointer);
        // fs.closeSync(fd);
        // function toBuffer(ab) {
        //   var buf = new Buffer(ab.byteLength);
        //   var view = new Uint8Array(ab);
        //   for (var i = 0; i < buf.length; ++i) {
        //     buf[i] = view[i];
        //   }
        //   return buf;
        // }
        // console.log(_this.downloadFileInfo.file_len);
        // console.log(Buf.length);
        fpointer += Buf.length;
        // 前端进度条 - 更新进度条
        if (downloading_id in this.downloading_file_map)
          this.setMap(
            "downloading_file_map",
            downloading_id,
            "cnt_now",
            fpointer
          );
        // console.log(fpointer);
        // break;
      }
      return 0;
    },
    async downloadFile(
      file_id,
      file_name,
      file_md5,
      local_path,
      downloading_dir_id
    ) {
      // let _this = this;
      // console.log(file_id);
      // console.log(file_name);
      // console.log(file_md5);
      // console.log(local_path);
      // console.log(downloading_dir_id);
      // console.log("i am in");
      // let file_name = "E:/mytest.txt";
      //拼接生成本地文件路径（下载到用户本地的哪里）
      // file_name = this.cur_dir + file_name;
      // let file_path = local_path + "/" + file_name;
      let fPath = path.join(local_path, file_name);
      let file_path = this.$common.replaceEscape(fPath);
      console.log(file_path);

      if (fs.existsSync(file_path)) {
        let md5 = this.$common.getFileMd5(file_path);
        console.log(md5);
        console.log(file_md5);
        if (md5 == file_md5) {
          console.log("The file exists.");
          console.log("file download complete...");
          this.$message.success(file_name+"秒下成功");
          return -3;
        } else {
          console.log("交给lq");
        }
      }
      console.log("new file download...");
      //first
      let downloading_id = await this.downloadFileFirst(
        file_id,
        this.$common.replaceEscape(local_path),
        downloading_dir_id
      );
      if (downloading_id == -1) return -1;

      //打开本地文件
      let fd = fs.openSync(file_path, "a");
      const stat = fs.statSync(file_path);
      console.log(stat.size);
      let ret = await this.downloadFileNormal(fd, stat.size, downloading_id);
      if (ret != 0) {
        fs.closeSync(fd);
        this.$common.setMap("downloading_file_map", downloading_id, "is_pause", true);
        return ret;
      }
      fs.closeSync(fd);
      //下载文件完毕
      return await this.$api
        .reqDownloadFileOver({
          downloading_id: downloading_id,
        })
        .then((data) => {
          if (data.msg == "success") {
            console.log("download file success!");
            if(downloading_dir_id != -1)
              this.downloading_dir_map[downloading_dir_id].cnt_now = data.file_now;
            // 清除待下载表项
            this.delMap("downloading_file_map", downloading_id);
            return 0;
          } else {
            console.log("download file error!");
            return -1;
          }
        });
      // return 0;
    },
    async downloadDirFirst(file_id, file_real_path) {
      let _this = this;
      return await this.$api
        .reqDownloadDirFirst({
          u_id: _this.u_id,
          file_real_path: file_real_path,
          logic_id: file_id,
        })
        .then(async(data) => {
          if (data.msg == "success") {
            // 前端进度条 - 更新前端vuex待下载文件列表
            if (Number(data.file_now) == 0) {
              let upl = await this.getDownDirList();
              for (let key in upl) {
                if (!(key in this.downloading_dir_map)) {
                  upl[key].is_pause = false;
                  this.addMap("downloading_dir_map", upl[key]);
                  break;
                }
              }
            }
            // _this.downloadDirInfo.downloading_id = data.downloading_id;
            // _this.downloadDirInfo.file_len = data.file_now;
            // _this.downloadDirInfo.slice_len = data.file_count;
            // _this.downloadDirInfo.stop_flag = 0;
            // _this.downloadDirInfo.file_cnt = 0;
            console.log("download-dir first success!");
            return data.downloading_id;
          } else {
            console.log("download-dir first error!");
            return -1;
          }
        });
    },
    async downloadDir(file_node, local_path, downloading_dir_id) {
      // let _this = this;
      let download_list;
      await this.$api
        .reqAllFileOfDir({
          file_node,
        })
        .then((res) => {
          console.log("reqAllFileOfDir", res);
          download_list = res;
        })
        .catch((err) => {
          console.log("reqAllFileOfDir", err);
        });

      if (fs.existsSync(local_path)) {
        console.log("dir ", local_path, " exist...");
      } else {
        //创建本地文件夹
        fs.mkdirSync(local_path, false);
      }

      let download_next_list = [];
      //遍历文件夹下的所有文件/文件夹
      for (let i = 0; i < download_list.length; i++) {
        //是文件夹，则放入next_list
        if (download_list[i].is_dir == 1) {
          download_next_list.push(download_list[i]);
        } //是文件，直接下载
        else {
          // if(this.downloadDirInfo.file_cnt < this.downloadDirInfo.file_now)
          // {
          //   console.log("file already downloaded...");
          //   this.downloadDirInfo.file_cnt = this.downloadDirInfo.file_cnt + 1;
          // }
          let ret = await this.downloadFile(
            download_list[i].id,
            download_list[i].file_name,
            download_list[i].md5,
            local_path,
            downloading_dir_id
          );
          if (ret == -1)
          {
            this.$common.setMap("downloading_dir_map", downloading_dir_id, "is_pause", true);
            return ret;
          }
          if (this.downloading_dir_map[downloading_dir_id].is_pause)
          {
            return -2;
          }
        }
      }
      //遍历文件夹，继续下载
      for (let k = 0; k < download_next_list.length; k++) {
        // local_path += "/" + dir_name;
        let fPath = path.join(local_path, download_next_list[k].file_name);
        let now_path = this.$common.replaceEscape(fPath);
        console.log(now_path);
        // await this.downloadDirFirst(download_next_list[k].id, now_path);
        let ret = await this.downloadDir(download_next_list[k].id, now_path, downloading_dir_id);
        if (ret != 0) return ret;
      }
      return 0;
    },
    async downloadDirTop(file_id, local_path, dir_name) {
      // local_path += "/" + dir_name;
      // this.downloadDirInfo.top_dir_id = file_id;
      let fPath = path.join(local_path, dir_name);
      let dir_path = this.$common.replaceEscape(fPath);
      console.log(dir_path);
      let downloading_id = await this.downloadDirFirst(file_id,this.$common.replaceEscape(local_path));
      if (downloading_id == -1) return -1;
      this.$message.success("开始下载"+dir_name);
      let ret = await this.downloadDir(file_id, dir_path, downloading_id);
      if (ret != 0) return ret;
      //文件夹下载完成
      return await this.$api
        .reqDownloadDirOver({
          downloading_id: downloading_id,
        })
        .then((data) => {
          if (data.msg == "success") {
            console.log("download dir success!");
            // 清除待下载表项
            this.delMap("downloading_dir_map", downloading_id);
            return 0;
          } else {
            console.log("download dir error!");
          return -1;
          }
        });
      // return 0;
    },

    // async refreshDownloadingList() {
    //   // 取得所有下载信息表
    //   await this.getAllUpDirList();
    //   await this.getAllUpFileList();
    //   let downloading_dir_list = this.downloading_dir_list;
    //   let downloading_file_list = this.downloading_file_list;
    //   console.log("dirlist", downloading_dir_list);
    //   console.log("flist", downloading_file_list);
    //   let download_obj = {};
    //   downloading_dir_list.forEach((element) => {
    //     element.cnt_now = element.file_now;
    //     element.cnt_target = element.file_count;
    //     element.is_dir = true;
    //     element.is_pause = true;
    //     download_obj[element.id] = element;
    //   });
    //   downloading_file_list.forEach((element) => {
    //     if (element.downloading_dir_id != -1) return;
    //     element.cnt_now = element.file_pointer;
    //     element.cnt_target = element.file_len;
    //     element.is_dir = false;
    //     element.is_pause = true;
    //     download_obj[element.id] = element;
    //   });
    //   return download_obj;
    // },
  },
};
</script>

<style lang="scss" scoped>
.container {
  display: flex;
  position: relative;
  .mask {
    width: 100%;
    height: 100%;
    position: fixed;
    .myalert {
      position: fixed;
      top: 40vh;
      left: 20vw;
      height: 20vh;
      width: 60vw;
    }
  }
  .aside {
    padding: 15px;
    background-color: pink;
    width: 220px;
    box-sizing: border-box;
    height: 100vh;
    flex-shrink: 0;
    .tab-button {
      // background-color: rgb(251, 240, 241);
      border-radius: 5px;
      font-size: 1.4rem;
      // margin-bottom: ;
      padding: 10px;
      cursor: pointer;
      &:hover {
        background-color: rgb(252, 219, 222);
      }
    }
    .hover-tab-button {
      background-color: rgb(251, 240, 241);
      &:hover {
        background-color: rgb(251, 240, 241);
      }
    }
  }
  .main {
    padding: 30px;
    width: 100%;
  }
}
</style>