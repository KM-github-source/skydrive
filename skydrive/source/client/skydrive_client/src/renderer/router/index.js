import Vue from 'vue'
import Router from 'vue-router'

Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/',
      name: 'login',
      component: require('@/views/Login').default
    },
    {
      // path: '/',
      path: '/register',
      name: 'register',
      component: require('@/views/Register').default
    },
    {
      path: '/home',
      name: 'home',
      component: require('@/views/Home').default,
      children: [
        {
          path: '/home/file',
          name: 'file',
          component: require('@/views/File').default,
        },
        {
          path: '/home/uploading',
          name: 'uploading',
          component: require('@/views/Uploading').default,
        },
        {
          path: '/home/downloading',
          name: 'downloading',
          component: require('@/views/Downloading').default,
        },
      ]
    },
    // {
    //   path: '/',
    //   name: 'landing-page',
    //   component: require('@/components/LandingPage').default
    // },
    {
      path: '*',
      redirect: '/'
    }
  ]
})
