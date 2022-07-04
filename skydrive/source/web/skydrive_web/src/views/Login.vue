<template>
  <div class="login-container">
    <div class="login-area">
      <div class="head">欢迎使用网盘</div>
      <div class="innerbox">
        <span class="note">用户名:</span>
        <el-input
          style="width: 75%"
          v-model="user_name"
          placeholder="请输入用户名"
        ></el-input>
      </div>
      <div class="innerbox">
        <span class="note">密码:</span>
        <el-input
          style="width: 75%"
          v-model="user_passwd"
          placeholder="请输入密码"
          show-password
        ></el-input>
      </div>
      <div class="button-box">
        <el-button class="button" @click="login" type="success">登录</el-button>
        <router-link to="/register">去注册</router-link>
      </div>
    </div>
  </div>
</template>

<script>
import md5 from "blueimp-md5";
import { mapMutations } from "vuex";

export default {
  data() {
    return {
      user_name: "",
      user_passwd: "",
      userinfo: {},
    };
  },
  methods: {
    ...mapMutations("Main", ["changeUid", "changeRootid", "changeUsername"]),
    login() {
      let hash = md5(this.user_passwd);
      let userdata = {
        user_name: this.user_name,
        passwd: hash,
        login_time: Date.now(),
      };
      console.log(userdata);
      this.$api.reqUserLogin(userdata).then((res) => {
        console.log(res);
        if (res.msg == "passwd error") {
          this.$message.error("密码错误");
        } else if (res.msg == "user not found") {
          this.$message.error("用户不存在");
        } else if (res.msg == "query error") {
          this.$message.error("查询错误");
        } else if (res.msg == "login success") {
          this.$message.success("登陆成功");
          sessionStorage.setItem("u_id", res.user_id);
          sessionStorage.setItem("root_id", res.root_id);
          sessionStorage.setItem("user_name", this.user_name);
          this.$router.replace({ name: "home" });
        }
      });
    },
  },
};
</script>

<style lang="scss" scoped>
.login-container {
  height: 100vh;
  width: 100vw;
  display: flex;
  justify-content: center;
  align-items: center;
  .login-area {
    width: 400px;
    height: 300px;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: space-around;
    border: rgba(0, 0, 0, 0.247) solid;
    border-radius: 10px;
    .head {
      font-size: 2rem;
      font-weight: bold;
    }
    .innerbox {
      width: 80%;
      margin: 0 10%;
      //   background-color: pink;
      display: flex;
      align-items: center;
      .note {
        width: 25%;
      }
    }
    .button-box {
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      .button {
        font-size: 1.3rem;
      }
    }
  }
}
</style>
