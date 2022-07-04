/<template>
  <div class="container">
    <div class="aside">
      <div class="tab-button">
        <i class="el-icon-tickets"></i>
        <span>文件</span>
      </div>
      <div class="tab-button" @click="logout">
        <i class="el-icon-circle-close"></i>
        <span>登出</span>
      </div>
    </div>
    <div class="main">
      <div class="head">
        <h1>欢迎，{{ user_name }}</h1>
        <div class="button">
          <el-tooltip effect="dark" content="刷新" placement="top">
            <i
              class="el-icon-refresh-right myiconbutton"
              @click="onclickRefresh"
            ></i>
          </el-tooltip>
          <el-tooltip effect="dark" content="新建" placement="top">
            <i
              class="el-icon-circle-plus-outline myiconbutton"
              @click="onclickMkdir"
            ></i>
          </el-tooltip>
          <el-tooltip effect="dark" content="上传" placement="top">
            <i
              class="el-icon-upload2 myiconbutton"
              @click="onClickUploadFile"
            ></i>
          </el-tooltip>
          <el-tooltip effect="dark" content="粘贴" placement="top">
            <i
              class="el-icon-document-copy myiconbutton"
              @click="onClickPaste"
            ></i>
          </el-tooltip>
          <el-tooltip effect="dark" content="移入" placement="top">
            <i
              class="el-icon-bottom-left myiconbutton"
              @click="onClickMovein"
            ></i>
          </el-tooltip>
        </div>
      </div>
      <div class="dir-now">
        当前位置：
        <el-breadcrumb separator="/">
          <el-breadcrumb-item
            class="mybreaditem"
            v-for="(item, index) in path_list"
            :key="item.id"
            ><span @click="chooseBread(index)">{{ item.file_name }}</span>
          </el-breadcrumb-item>
        </el-breadcrumb>
      </div>
      <div class="file-list">
        <div class="item" v-for="item in file_list" :key="item.id">
          <i
            class="myicon"
            :class="
              item.is_dir == 0 ? 'el-icon-document' : 'el-icon-folder-opened'
            "
          ></i>
          <div class="name" @click="enterDir(item)">{{ item.file_name }}</div>
          <div class="button">
            <el-tooltip effect="dark" content="重命名" placement="top">
              <i
                class="el-icon-edit myiconbutton"
                @click="onClickChangeName(item)"
              ></i>
            </el-tooltip>
            <el-tooltip effect="dark" content="下载" placement="top">
              <i
                class="el-icon-download myiconbutton"
                @click="onClickDownload(item)"
              ></i>
            </el-tooltip>
            <el-tooltip effect="dark" content="复制" placement="top">
              <i
                class="el-icon-copy-document myiconbutton"
                @click="onClickCopy(item)"
              ></i>
            </el-tooltip>
            <el-tooltip effect="dark" content="删除" placement="top">
              <i
                class="el-icon-delete myiconbutton"
                @click="onClickDelete(item)"
              ></i>
            </el-tooltip>
            <el-tooltip effect="dark" content="移动" placement="top">
              <i
                class="el-icon-right myiconbutton"
                @click="onClickMove(item)"
              ></i>
            </el-tooltip>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { mapState, mapMutations } from "vuex";
import SparkMD5 from "spark-md5";

async function getFileMd5(fileObject) {
  return new Promise(async (resolve, reject) => {
    try {
      const spark = new SparkMD5.ArrayBuffer();
      const fileReader = new FileReader();
      // 异步操作，读完后的结果
      fileReader.onload = (e) => {
        spark.append(e.target.result);
        const fileMd5 = spark.end();
        return resolve({ [fileObject.name]: fileMd5 });
      };
      fileReader.readAsArrayBuffer(fileObject);
    } catch (err) {
      return reject(err);
    }
  });
}

export default {
  data() {
    return {
      u_id: "",
      root_id: -1,
      user_name: "",
      input_data: "",
      path_list: [],
      file_list: [],
    };
  },
  computed: {
    ...mapState(["copy_info", "move_info"]),
  },
  mounted() {
    this.u_id = sessionStorage.getItem("u_id");
    this.root_id = sessionStorage.getItem("root_id");
    this.user_name = sessionStorage.getItem("user_name");
    let root_id = this.root_id;
    this.path_list = [
      {
        file_name: "root",
        id: root_id,
      },
    ];
    this.changeDir(root_id);
  },
  methods: {
    ...mapMutations(["changeCopyinfo", "changeMoveInfo"]),
    logout() {
      // 清空用户信息
      this.$confirm("确认登出？", "提示", {
        confirmButtonText: "确定",
        cancelButtonText: "取消",
        type: "warning",
      }).then(() => {
        sessionStorage.clear();
        this.$router.replace({ name: "login" });
      });
    },
    changeDir(file_node) {
      return this.$api
        .reqAllFileOfDir({
          file_node,
        })
        .then((res) => {
          console.log("this.$api.reqAllFileOfDir", res);
          this.file_list = res;
        })
        .catch((err) => {
          console.log("this.$api.reqAllFileOfDir", err);
        });
    },
    enterDir(item) {
      if (item.is_dir == 0) {
        // 下载
        this.onClickDownload(item);
        return;
      }
      this.changeDir(item.id);
      this.path_list.push({
        file_name: item.file_name,
        id: item.id,
      });
    },
    chooseBread(index) {
      if (index == this.path_list.length - 1) return;
      console.log(this.path_list[index]);
      this.path_list.splice(index + 1, this.path_list.length - index - 1);
      this.changeDir(this.path_list[index].id);
    },
    async getInput(prompt) {
      await this.$prompt(prompt, "提示", {
        confirmButtonText: "确定",
        cancelButtonText: "取消",
        inputPattern: /.+/,
        inputErrorMessage: "您尚未输入内容",
        closeOnClickModal: false,
      })
        .then(({ value }) => {
          this.input_data = value;
        })
        .catch(() => {
          this.input_data = "";
        });
    },
    refresh() {
      this.changeDir(this.path_list[this.path_list.length - 1].id);
    },
    onclickRefresh() {
      this.refresh();
      this.$message({
        message: "刷新成功",
        type: "success",
      });
    },
    async onclickMkdir() {
      await this.getInput("请输入新建文件夹名");
      let path_name = this.input_data;
      if (path_name == "") return;
      for (let i = 0; i < this.file_list.length; i++) {
        console.log("文件夹检查", this.file_list[i]);
        if (path_name == this.file_list[i].file_name) {
          this.$message.error("该目录存在同名文件,请修改名称");
          return;
        }
      }
      let ret = await this.$api.reqUploadMkdir({
        file_name: path_name,
        parent_id: this.path_list[this.path_list.length - 1].id,
      });
      if (ret.msg == "success") {
        this.refresh();
        this.$message.success("新建文件夹成功");
        return;
      } else {
        this.$message.error("新建文件夹失败");
        return;
      }
    },
    // 要改
    async onClickUploadFile() {
      // this.$message("点击上传");
      // return;
      // 上传文件
      let input = document.createElement("input");
      input.type = "file";
      input.click();
      input.onchange = async () => {
        let file = input.files[0];
        console.log("file", file);
        let file_md5 = await getFileMd5(file);
        let file_name = file.name;
        file_md5 = file_md5[file_name];
        let same_file_i = -1;
        for (let i = 0; i < this.file_list.length; i++)
          if (this.file_list[i].file_name === file_name) {
            same_file_i = i;
            break;
          }
        if (same_file_i !== -1) {
          // 弹窗提示
          let confirm = await this.$confirm(
            "检查到云端有同名文件, 是否覆盖?",
            "提示",
            {
              confirmButtonText: "确定",
              cancelButtonText: "取消",
              type: "warning",
            }
          )
            .then(() => {
              this.deleteFile(this.file_list[same_file_i].id);
              return true;
            })
            .catch(() => {
              return false;
            });
          if (!confirm) return;
        }
        let form_data = new FormData();
        console.log("md5", file_md5);
        form_data.append("md5", file_md5);
        form_data.append("file_name", file_name);
        form_data.append("file_size", file.size);
        form_data.append(
          "parent_id",
          this.path_list[this.path_list.length - 1].id
        );
        form_data.append("file_content", file);
        // 首先，查看是否可以秒传
        let res = await this.$api.reqUploadCheckFileExist({
          md5: file_md5,
        });
        let parent_id = this.path_list[this.path_list.length - 1].id;
        if (res.is_exist == 1) {
          res = await this.$api.reqUploadFileFast({
            parent_id,
            file_name,
            md5: file_md5,
          });
          if (res.msg === "success") {
            console.log(file_name, "秒传成功");
            this.$message.success(file_name + " 秒传成功");
          } else {
            console.log(file_name, "秒传失败");
            this.$message.success(file_name + " 秒传失败");
          }
          return 0;
        }
        this.$message.success("开始上传");
        this.$api.reqWebUpload(form_data).then((res) => {
          if (res.msg == "success") {
            this.$message.success("文件上传成功");
            this.refresh();
          } else this.$message.error("上传错误，请重试");
        });
      };
    },
    // 要改
    async onClickDownload(item) {
      this.$api
        .reqWebDownload({
          id: item.id,
        })
        .then((res) => {
          console.log("res", res);
          const ele = document.createElement("a");
          ele.setAttribute("href", URL.createObjectURL(res)); //设置下载文件的url地址
          ele.setAttribute("download", item.file_name);
          ele.click();
          URL.revokeObjectURL(ele.href);
          document.body.removeChild(ele);
        })
        .catch((err) => {
          console.log("err", err);
        });
    },
    onClickCopy(item) {
      this.changeCopyinfo({
        id: item.id,
        is_dir: item.is_dir,
        parent_id: this.path_list[this.path_list.length - 1].id,
      });
      this.$message({
        message: "文件信息拷贝成功，请至对应页面点击“粘贴”按钮",
        type: "success",
      });
    },
    deleteFile(id) {
      return this.$api.reqDelFile({ id });
    },
    async onClickDelete(item) {
      let confirm = await this.$confirm(
        "确认删除" + item.file_name + "吗？",
        "提示",
        {
          confirmButtonText: "确定",
          cancelButtonText: "取消",
          type: "warning",
        }
      )
        .then(() => {
          return true;
        })
        .catch(() => {
          return false;
        });
      if (!confirm) return;
      let ret = await this.$api.reqDelFile({ id: item.id });
      if (ret.msg == "success") {
        this.$message({
          message: item.file_name + "删除成功",
          type: "success",
        });
      } else {
        this.$message({
          message: item.file_name + "删除失败",
          type: "error",
        });
      }
      this.refresh();
    },
    onClickMove(item) {
      this.changeMoveInfo({
        id: item.id,
        parent_id: this.path_list[this.path_list.length - 1].id,
      });
      this.$message({
        message: "文件信息记录成功，请至对应页面点击“移入”按钮",
        type: "success",
      });
    },
    // 检查当前所在目录是否是原来的子目录
    checkInclude(old_id) {
      for (let i = 0; i < this.path_list.length; i++)
        if (old_id == this.path_list[i].id) return true;
      return false;
    },
    onClickPaste() {
      let new_parent_id = this.path_list[this.path_list.length - 1].id;
      console.log("npi", this.copy_info.id);
      if (!this.copy_info.id) {
        this.$message.error("尚未复制文件");
        return;
      } else if (new_parent_id == this.copy_info.parent_id) {
        this.$message.error("待粘贴的目录与原目录一致");
        return;
      } else if (this.checkInclude(this.copy_info.id)) {
        this.$message.error("待粘贴的目录是源文件夹的子目录");
        return;
      }

      let prom =
        this.copy_info.is_dir == 1
          ? this.$api.reqCopyDir({
              id: this.copy_info.id,
              new_parent_id,
            })
          : this.$api.reqCopyFile({
              id: this.copy_info.id,
              new_parent_id,
            });
      prom.then((res) => {
        if (res.msg === "success") {
          this.refresh();
          this.changeCopyinfo({});
          this.$message({
            message: "粘贴成功",
            type: "success",
          });
        } else this.$message.error("粘贴错误");
      });
    },
    onClickMovein() {
      let new_parent_id = this.path_list[this.path_list.length - 1].id;
      if (!this.move_info.id) {
        this.$message.error("尚未选择待移入的文件");
        return;
      } else if (new_parent_id == this.move_info.parent_id) {
        this.$message.error("待移入的目录与原目录一致");
        return;
      } else if (this.checkInclude(this.move_info.id)) {
        this.$message.error("待移入的目录是源文件夹的子目录");
        return;
      }

      this.$api
        .reqMoveFile({
          id: this.move_info.id,
          new_parent_id,
        })
        .then((res) => {
          if (res.msg === "success") {
            this.changeMoveInfo({});
            this.refresh();
            this.$message({
              message: "移入成功",
              type: "success",
            });
          }
        });
    },
    async onClickChangeName(item) {
      await this.getInput("请输入新名称");
      let new_name = this.input_data;
      if (new_name == "") return;
      for (let i = 0; i < this.file_list.length; i++) {
        console.log("文件夹检查", this.file_list[i]);
        if (new_name == this.file_list[i].file_name) {
          this.$message.error("该目录存在同名文件,请修改名称");
          return;
        }
      }
      let ret = await this.$api.reqChangeName({
        id: item.id,
        newfilename: new_name,
      });
      if (ret.msg == "success") {
        this.refresh();
        this.$message.success("修改名称成功");
        return;
      } else {
        this.$message.error("修改名称失败");
        return;
      }
    },
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
    height: 100%;
    // font-size: 2rem;
    .head {
      display: flex;
      align-items: center;
      width: 100%;
      .button {
        margin-left: auto;
        margin-right: 20px;
        font-size: 1.6rem;
        .myiconbutton {
          border: 1px solid #aaa;
          border-radius: 50%;
          padding: 5px;
          margin-right: 10px;
          &:hover {
            background-color: rgba($color: pink, $alpha: 0.3);
            cursor: pointer;
          }
        }
      }
    }
    .dir-now {
      margin-top: 10px;
      display: flex;
      align-items: center;
      .mybreaditem {
        font-size: 1.2rem;
        span {
          &:hover {
            cursor: pointer;
            color: rgb(67, 138, 244);
            text-decoration: underline;
          }
        }
      }
    }
    .file-list {
      font-size: 1.2rem;
      margin-top: 20px;
      width: 100%;
      border-left: 10px solid pink;
      max-height: calc(95vh - 150px);
      overflow-y: scroll;
      .item {
        display: flex;
        align-items: center;
        margin-bottom: 8px;
        margin-right: 30px;
        padding: 4px 20px;
        // border: 1px solid #aaa;
        .myicon {
          color: brown;
        }
        .name {
          &:hover {
            color: rgb(67, 138, 244);
            cursor: pointer;
            text-decoration: underline;
          }
        }
        .button {
          margin-left: auto;
          display: flex;
          .myiconbutton {
            display: block;
            margin: 0 10px;
            &:hover {
              background-color: rgba($color: pink, $alpha: 0.7);
              cursor: pointer;
            }
          }
        }
        &:hover {
          background-color: rgba($color: pink, $alpha: 0.3);
        }
      }
    }
  }
}
</style>