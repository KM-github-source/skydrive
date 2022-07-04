import Vue from 'vue'
import Vuex from 'vuex'

Vue.use(Vuex)

export default new Vuex.Store({
  state: {
    copy_info: {},
    move_info: {},
  },
  mutations: {
    changeCopyinfo(state, copy_info) {
      state.copy_info = copy_info
    },
    changeMoveInfo(state, move_info) {
      state.move_info = move_info
    },
  },
  getters: {
    copy_info: state => state.copy_info,
    move_info: state => state.move_info,
  }
})
