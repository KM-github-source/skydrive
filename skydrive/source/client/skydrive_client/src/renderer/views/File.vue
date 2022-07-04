/<template>
  <div class="file-container">
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
</template>

<script>
import { mapState, mapMutations } from "vuex";
const fs = require("fs");

export default {
  data() {
    return {
      input_data: "",
      path_list: [],
      file_list: [],
    };
  },
  mounted() {
    let root_id = this.root_id;
    this.path_list = [
      {
        file_name: "root",
        id: root_id,
      },
    ];
    this.changeDir(root_id);
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
    uploading_file_map() {
      return this.$common.getMap("uploading_file_map");
    },
  },
  methods: {
    ...mapMutations("Main", [
      "changeUid",
      "changeRootid",
      "changeUsername",
      "changeCopyinfo",
      "changeMoveInfo",
      // "changeUploadingList",
      // "changeDownloadingList",
      // "addUploadingItem",
      // "delUploadingItem",
      // "updateUploadingItem",
    ]),
    changeDir(file_node) {
      console.log("changeDir", file_node);
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
    showError(prompt) {
      this.$common.showError(prompt);
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
    async onclickMkdir() {
      await this.getInput("请输入该目录下要新建的文件夹");
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
    async onClickUploadFile() {
      // D:\Mydata\homework\大三下\课程\计算机网络\小组作业\网盘\test\dddd
      await this.getInput("请输入本地要上传的文件（或文件夹）路径");
      let file_path = this.input_data;
      let stat;
      let ret;
      try {
        stat = fs.statSync(file_path);
      } catch (err) {
        this.$message.error(
          "上传的路径不正确，可能是文件不存在或权限不足，请检查"
        );
        return;
      }
      // 取文件名
      let file_name = this.$common.getFileNameByPath(file_path);
      // 查找云端是否有同名，有则提示是否覆盖
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
      this.$message({
        message: "开始上传，请在上传列表查看传输情况",
        type: "success",
      });
      // 区分是文件夹还是文件
      if (!stat.isDirectory())
        ret = await this.$common.uploadFile(
          file_path,
          this.path_list[this.path_list.length - 1].id,
          -1
        );
      else
        ret = await this.$common.uploadDir(
          file_path,
          file_name,
          this.path_list[this.path_list.length - 1].id
        );
      this.refresh();
      console.log("onclick upl file", ret);
      if (ret == 0) {
        this.$message.success(file_name + "上传成功");
      }
    },
    async onClickDownload(item) {
      await this.getInput("请输入要下载到本地的文件路径（不含文件名）");
      let file_path = this.input_data;
      if (file_path == "") {
        this.$message.error("尚未输入下载路径！");
        return;
      }
      let ret;
      // 区分是文件夹还是文件
      if (item.is_dir == 1) {
        ret = await this.$common.downloadDirTop(
          item.id,
          file_path,
          item.file_name
        );
        // let download_map =
        // item.is_dir == 1 ? "downloading_dir_map" : "downloading_file_map";
        // console.log("item id:", item);
        // this.$common.setMap(download_map, item.id, "is_pause", true);
      } else
        ret = await this.$common.downloadFile(
          item.id,
          item.file_name,
          item.md5,
          file_path,
          -1
        );
      if (ret == -1) {
        this.$message.error("下载文件出错，请重试");
        return;
      } else if (ret == 0) {
        this.$message.success(item.file_name + " 下载完成");
      }
      this.refresh();
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
  },
};
</script>

<style lang="scss" scoped>
.file-container {
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
    overflow-y: scroll;
    max-height: calc(80vh - 100px);
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
</style>