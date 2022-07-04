/*
Navicat MySQL Data Transfer

Source Server         : jw2022
Source Server Version : 50505
Source Host           : 139.9.243.37:3306
Source Database       : skydrive

Target Server Type    : MYSQL
Target Server Version : 50505
File Encoding         : 65001

Date: 2022-07-02 23:18:08
*/

drop database if exists skydrive;
create database skydrive;
use skydrive;
SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for downloading_dir
-- ----------------------------
DROP TABLE IF EXISTS `downloading_dir`;
CREATE TABLE `downloading_dir` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `u_id` int(11) DEFAULT NULL,
  `file_real_path` varchar(255) DEFAULT NULL,
  `md5` varchar(255) DEFAULT NULL,
  `logic_id` int(11) DEFAULT NULL,
  `file_now` int(11) DEFAULT NULL,
  `file_count` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=182 DEFAULT CHARSET=gbk;

-- ----------------------------
-- Table structure for downloading_file
-- ----------------------------
DROP TABLE IF EXISTS `downloading_file`;
CREATE TABLE `downloading_file` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `u_id` int(11) DEFAULT NULL,
  `logic_id` int(11) DEFAULT NULL,
  `file_len` bigint(20) DEFAULT NULL,
  `file_real_path` varchar(255) DEFAULT NULL,
  `downloading_dir_id` int(11) DEFAULT NULL,
  `file_pointer` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1238 DEFAULT CHARSET=gbk;

-- ----------------------------
-- Table structure for logic_file
-- ----------------------------
DROP TABLE IF EXISTS `logic_file`;
CREATE TABLE `logic_file` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `file_name` varchar(255) DEFAULT NULL,
  `md5` varchar(255) DEFAULT NULL,
  `parent_id` int(11) DEFAULT NULL,
  `is_dir` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=598 DEFAULT CHARSET=gbk;

-- ----------------------------
-- Table structure for physic_file
-- ----------------------------
DROP TABLE IF EXISTS `physic_file`;
CREATE TABLE `physic_file` (
  `md5` varchar(255) NOT NULL,
  `real_path` varchar(255) DEFAULT NULL,
  `file_size` bigint(20) DEFAULT NULL,
  `file_link_cnt` int(11) DEFAULT NULL,
  PRIMARY KEY (`md5`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;

-- ----------------------------
-- Table structure for uploading_dir
-- ----------------------------
DROP TABLE IF EXISTS `uploading_dir`;
CREATE TABLE `uploading_dir` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `u_id` int(11) DEFAULT NULL,
  `file_real_path` varchar(255) DEFAULT NULL,
  `md5` varchar(255) DEFAULT NULL,
  `logic_id` int(11) DEFAULT NULL,
  `file_now` int(255) DEFAULT NULL,
  `file_count` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=77 DEFAULT CHARSET=gbk;

-- ----------------------------
-- Table structure for uploading_file
-- ----------------------------
DROP TABLE IF EXISTS `uploading_file`;
CREATE TABLE `uploading_file` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `u_id` int(11) DEFAULT NULL,
  `file_real_path` varchar(255) DEFAULT NULL,
  `md5` varchar(255) DEFAULT NULL,
  `file_len` bigint(20) DEFAULT NULL,
  `file_pointer` bigint(20) DEFAULT NULL,
  `parent_id` int(11) DEFAULT NULL,
  `file_name` varchar(255) DEFAULT NULL,
  `uploading_dir_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=454 DEFAULT CHARSET=gbk;

-- ----------------------------
-- Table structure for user
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) DEFAULT NULL,
  `passwd` varchar(255) DEFAULT NULL,
  `root_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=21 DEFAULT CHARSET=gbk;

-- ----------------------------
-- Table structure for user_login_info
-- ----------------------------
DROP TABLE IF EXISTS `user_login_info`;
CREATE TABLE `user_login_info` (
  `user_id` int(11) NOT NULL,
  `ipaddr` varchar(255) DEFAULT NULL,
  `login_time` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;
