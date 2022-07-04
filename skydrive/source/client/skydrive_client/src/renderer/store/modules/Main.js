const state = {
  u_id: -1,
  root_id: 4,
  user_name: "李强",
  copy_info: {},
  move_info: {},
  // uploading_list: {},
  // uploading_dir_list: {},
  // downloading_list: {},
  // downloading_dir_list: {},
}

const getters = {
  u_id: state => state.u_id,
  root_id: state => state.root_id,
  user_name: state => state.user_name,
  copy_info: state => state.copy_info,
  move_info: state => state.move_info,
  // uploading_list: state => state.uploading_list,
  // uploading_dir_list: state => state.uploading_dir_list,
  // downloading_list: state => state.downloading_list,
  // downloading_dir_list: state => state.downloading_dir_list,
}

const mutations = {
  changeUid(state, u_id) {
    state.u_id = u_id
  },
  changeRootid(state, root_id) {
    state.root_id = root_id
  },
  changeUsername(state, user_name) {
    state.user_name = user_name
  },
  changeCopyinfo(state, copy_info) {
    state.copy_info = copy_info
  },
  changeMoveInfo(state, move_info) {
    state.move_info = move_info
  },
  // changeUploadingList(state, uploading_list) {
  //   state.uploading_list = { ...uploading_list }
  // },
  // addUploadingItem(state, item) {
  //   state.uploading_list = {
  //     ...state.uploading_list,
  //     [item.id]: item
  //   }
  // },
  // delUploadingItem(state, id) {
  //   let tmp = {}
  //   for (let key in state.uploading_list)
  //     if (key != id)
  //       tmp[key] = state.uploading_list[key]
  //   state.uploading_list = tmp
  // },
  // updateUploadingItem(state, { id, key, value }) {
  //   console.log(arguments)
  //   let tmp = { ...state.uploading_list }
  //   tmp[id][key] = value
  //   state.uploading_list = tmp
  // },
  // changeUploadingDirList(state, uploading_list) {
  //   state.uploading_dir_list = { ...uploading_dir_list }
  // },
  // addUploadingDirItem(state, item) {
  //   state.uploading_dir_list = {
  //     ...state.uploading_dir_list,
  //     [item.id]: item
  //   }
  // },
  // delUploadingDirItem(state, id) {
  //   let tmp = {}
  //   for (let key in state.uploading_dir_list)
  //     if (key != id)
  //       tmp[key] = state.uploading_dir_list[key]
  //   state.uploading_dir_list = tmp
  // },
  // updateUploadingDirItem(state, { id, key, value }) {
  //   console.log(arguments)
  //   let tmp = { ...state.uploading_dir_list }
  //   tmp[id][key] = value
  //   state.uploading_dir_list = tmp
  // },
  // changeDownloadingList(state, downloading_list) {
  //   state.downloading_list = { ...downloading_list }
  // },
}

export default {
  namespaced: true,
  state,
  mutations,
  getters,
}
