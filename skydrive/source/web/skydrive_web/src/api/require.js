import axios from "axios";

const requests = axios.create({
    baseURL: "http://139.9.243.37:40000",   // 服务器
    // baseURL: "https://mock.apifox.cn/m1/1181962-0-default",   // mock服务
    timeout: 5000,
});
requests.interceptors.request.use((config) => {
    return config;
});
requests.interceptors.response.use((res) => {
    console.log(res);
    return res.data;
}, (error) => {
    return error;
});
export default requests;