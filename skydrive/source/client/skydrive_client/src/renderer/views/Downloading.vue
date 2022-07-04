/<template>
  <div class="downloading-container">
    <div class="myh1">下载文件列表</div>
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
              <div
                class="button"
                v-show="
                  !scope.row.downloading_dir_id ||
                  scope.row.downloading_dir_id == -1
                "
              >
                <el-tooltip
                  effect="dark"
                  :content="scope.row.is_pause ? '继续下载' : '暂停下载'"
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
                {{ scope.row.is_pause ? "已暂停" : "正在下载" }}
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
      // "downloading_list",
      // "downloading_list",
    ]),
    tableData() {
      let ret = [];
      let dmp = this.$common.getMap("downloading_dir_map");
      let fmp = this.$common.getMap("downloading_file_map");
      for (let key in dmp) ret.push(dmp[key]);
      for (let key in fmp) ret.push(fmp[key]);
      return ret;
    },
  },
  methods: {
    // ...mapMutations("Main", ["delUploadingItem", "downdateUploadingItem"]),
    async pause(item) {
      let download_map =
        item.is_dir == 1 ? "downloading_dir_map" : "downloading_file_map";
      this.$common.setMap(download_map, item.id, "is_pause", !item.is_pause);
      console.log("pause", item);
      if (item.is_pause) {
        this.$message.info("下载已暂停");
        return;
      }
      let ret;
      if (item.is_dir == 0) {
        ret = await this.$common.downloadFile(
          item.logic_id,
          item.file_name,
          item.md5,
          item.file_real_path,
          item.downloading_dir_id
        );
      } else {
        ret = await this.$common.downloadDirTop(
          item.logic_id,
          item.file_real_path,
          item.file_name
        );
      }

      if (ret == -1) {
        this.$message.error("下载出错，请重试");
        return;
      } else if (ret == 0) {
        this.$message({
          message: item.file_name + "下载成功",
          type: "success",
        });
      }
    },
  },
  async mounted() {},
};
</script>

<style lang="scss" scoped>
.downloading-container {
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