<template>
  <div class="register-container">
    <div class="login-area">
      <div class="head">新用户注册</div>
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
          v-model="new_password"
          placeholder="请输入密码"
          show-password
        ></el-input>
      </div>
      <div class="innerbox">
        <span class="note">确认密码:</span>
        <el-input
          style="width: 75%"
          v-model="new_password2"
          placeholder="请输入密码"
          show-password
        ></el-input>
      </div>
      <div class="button-box">
        <el-button class="button" @click="changepasswd" type="success">注册</el-button>
        <router-link to="/">去登录</router-link>
      </div>
      <div class="notice">
        注意：密码长度大于12位，且必须同时包含数字/小写字母/大写字母/其它字符中三种
      </div>
    </div>
  </div>
</template>

<script>
import md5 from "blueimp-md5";
export default {
  data() {
    return {
      fun: 1,
      user_name: "",
      new_password: "",
      new_password2: "",
      userinfo: {},
    };
  },
  methods: {
    changepasswd() {
      if (this.new_password != this.new_password2) {
        this.$message.error("两次新密码密码不一致");
      } else if (this.new_password.length < 12) {
        this.$message.error("密码长度不符合要求");
      } else {
        let num = 0;
        let bigcha = 0;
        let smallcha = 0;
        let oth = 0;
        let count = 0;
        for (let i = 0; i < this.new_password.length; i++) {
          if (this.new_password[i] >= "0" && this.new_password[i] <= "9") {
            if (num == 0) {
              num = 1;
              count++;
            }
          } else if (
            this.new_password[i] >= "a" &&
            this.new_password[i] <= "z"
          ) {
            if (smallcha == 0) {
              smallcha = 1;
              count++;
            }
          } else if (
            this.new_password[i] >= "A" &&
            this.new_password[i] <= "Z"
          ) {
            if (bigcha == 0) {
              bigcha = 1;
              count++;
            }
          } else {
            if (oth == 0) {
              oth = 1;
              count++;
            }
          }
        }
        if (count < 3) {
          this.$message.error("密码格式不正确");
        } else {
          let regpack = {
            user_name: this.user_name,
            passwd: md5(this.new_password),
          };
          this.$api.reqUserRegister(regpack).then((res) => {
            if (res.msg == "sql query err") {
              this.$message.error("查询错误");
            } else if (res.msg == "user_name already exist") {
              this.$message.error("用户已存在");
            } else if (res.msg == "sql insert err") {
              this.$message.error("插入错误");
            } else if (res.msg == "success") {
              this.$message.success("注册成功!");
              this.$router.push("/login");
            }
          });
        }
      }
    },
  },
};
</script>

<style lang="scss" scoped>
.register-container {
  height: 100vh;
  width: 100vw;
  display: flex;
  justify-content: center;
  align-items: center;
  .login-area {
    width: 400px;
    height: 350px;
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
    .notice {
      font-size: 0.9rem;
      margin: 0 10%;
    }
  }
}
</style>
