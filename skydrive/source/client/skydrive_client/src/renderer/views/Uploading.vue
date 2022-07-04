/<template>
  <div class="uploading-container">
    <div class="myh1">上传文件列表</div>
    <div class="table">
      <el-table :data="tableData" style="width: 100%">
        <el-table-column label="名称">
          <template slot-scope="scope">
            <div class="file_info">
              <div class="file_name">
                <i
                  class="myicon"
                  :class="
                    scope.row.is_dir == 0
                      ? 'el-icon-document'
                      : 'el-icon-folder-opened'
                  "
                ></i>
                <span style="margin-left: 10px">{{ scope.row.file_name }}</span>
              </div>
              <div class="button">
                <el-tooltip
                  effect="dark"
                  :content="scope.row.is_pause ? '继续上传' : '暂停上传'"
                  placement="top"
                >
                  <i
                    @click="pause(scope.row)"
                    class="myiconbutton"
                    :class="
                      scope.row.is_pause
                        ? 'el-icon-video-play'
                        : 'el-icon-video-pause'
                    "
                  ></i>
                </el-tooltip>
              </div>
            </div>
          </template>
        </el-table-column>
        <el-table-column label="大小" width="200">
          <template slot-scope="scope">
            <div class="size">
              <span
                >{{ scope.row.cnt_now }} / {{ scope.row.cnt_target }}
                {{ scope.row.is_dir == 1 ? "项" : "B" }}</span
              >
            </div>
          </template>
        </el-table-column>
        <el-table-column label="状态" width="150">
          <template slot-scope="scope">
            <div class="progress">
              <div class="text">
                {{ scope.row.is_pause ? "已暂停" : "正在上传" }}
              </div>
              <el-progress
                :show-text="false"
                :percentage="
                  Math.floor((100 * scope.row.cnt_now) / scope.row.cnt_target)
                "
              ></el-progress>
            </div>
          </template>
        </el-table-column>
      </el-table>
    </div>
  </div>
</template>

<script>
import { mapState, mapMutations } from "vuex";

export default {
  data() {
    return {};
  },
  computed: {
    ...mapState("Main", [
      "u_id",
      "root_id",
      // "uploading_list",
      // "downloading_list",
    ]),
    tableData() {
      let ret = [];
      let dmp = this.$common.getMap("uploading_dir_map");
      let fmp = this.$common.getMap("uploading_file_map");
      for (let key in dmp) ret.push(dmp[key]);
      for (let key in fmp) ret.push(fmp[key]);
      return ret;
    },
  },
  methods: {
    // ...mapMutations("Main", ["delUploadingItem", "updateUploadingItem"]),
    async pause(item) {
      let upload_map =
        item.is_dir == 1 ? "uploading_dir_map" : "uploading_file_map";
      this.$common.setMap(upload_map, item.id, "is_pause", !item.is_pause);
      console.log("pause", item);
      console.log("pause", item.is_dir, item.is_dir == 0);
      if (item.is_pause) {
        this.$message.info("上传已暂停");
        return;
      }

      let ret;
      if (item.is_dir == 0) {
        ret = await this.$common.uploadFile(
          item.file_real_path,
          item.parent_id,
          item.uploading_dir_id
        );
      } else {
        ret = await this.$common.uploadDir(
          item.file_real_path,
          item.file_name,
          item.parent_id
        );
      }

      if (ret == -1) {
        this.$message.error("上传出错，请重试");
        return;
      }
      this.$message({
        message: item.file_name + "上传成功",
        type: "success",
      });
    },
  },
  async mounted() {},
};
</script>

<style lang="scss" scoped>
.uploading-container {
  width: 100%;
  height: 100%;
  .myh1 {
    font-size: 1.6rem;
  }
  .table {
    margin: 10px;
    overflow-y: scroll;
    height: calc(88vh - 50px);
    .file_info {
      font-size: 1.2rem;
      display: flex;
      align-items: center;
      .file_name {
        display: flex;
        align-items: center;
      }
      .button {
        margin-left: auto;
        margin-right: 10px;
        padding-left: 20px;
        .myiconbutton {
          font-size: 1.5rem;
          &:hover {
            cursor: pointer;
            color: rgb(248, 149, 166);
          }
        }
      }
    }
    .size {
      font-size: 1rem;
    }
    .progress {
      .text {
        font-size: 0.8rem;
      }
    }
  }
}
</style>