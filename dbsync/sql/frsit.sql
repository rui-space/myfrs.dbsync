/*
Navicat MySQL Data Transfer

Source Server         : 10.10.10.13
Source Server Version : 50726
Source Host           : 10.10.10.13:3306
Source Database       : frsit

Target Server Type    : MYSQL
Target Server Version : 50726
File Encoding         : 65001

Date: 2019-06-26 16:03:35
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for `auth_info`
-- ----------------------------
DROP TABLE IF EXISTS `auth_info`;
CREATE TABLE `auth_info` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '主键',
  `name` varchar(255) NOT NULL COMMENT '名称',
  `level` int(11) NOT NULL COMMENT '权限级别（数值小的权限高）',
  `desc` varchar(255) DEFAULT NULL COMMENT '权限描述',
  `create_time` datetime NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of auth_info
-- ----------------------------

-- ----------------------------
-- Table structure for `game_conf`
-- ----------------------------
DROP TABLE IF EXISTS `game_conf`;
CREATE TABLE `game_conf` (
  `id` int(11) NOT NULL,
  `show_qrcode` int(11) DEFAULT NULL COMMENT '是否显示二维码',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Records of game_conf
-- ----------------------------

-- ----------------------------
-- Table structure for `game_info`
-- ----------------------------
DROP TABLE IF EXISTS `game_info`;
CREATE TABLE `game_info` (
  `id` int(10) NOT NULL AUTO_INCREMENT COMMENT '索引',
  `name` varchar(255) DEFAULT NULL COMMENT '名称',
  `description` varchar(255) DEFAULT NULL COMMENT '游戏说明',
  `content` text COMMENT '详情',
  `thumbnail` varchar(255) DEFAULT NULL COMMENT '缩略图url',
  `install_path` varchar(255) DEFAULT NULL COMMENT '游戏安装路径',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of game_info
-- ----------------------------

-- ----------------------------
-- Table structure for `place`
-- ----------------------------
DROP TABLE IF EXISTS `place`;
CREATE TABLE `place` (
  `id` int(10) NOT NULL AUTO_INCREMENT COMMENT '索引',
  `name` varchar(255) DEFAULT NULL COMMENT '名称',
  `sn` varchar(255) DEFAULT NULL COMMENT '编号',
  `picture` varchar(255) DEFAULT NULL COMMENT '宣传片',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of place
-- ----------------------------

-- ----------------------------
-- Table structure for `project`
-- ----------------------------
DROP TABLE IF EXISTS `project`;
CREATE TABLE `project` (
  `id` int(10) NOT NULL AUTO_INCREMENT COMMENT '索引',
  `name` varchar(255) DEFAULT NULL COMMENT '名称',
  `sn` varchar(255) DEFAULT NULL COMMENT '编号',
  `player_count` int(10) DEFAULT NULL COMMENT '玩家数量',
  `place_id` int(10) DEFAULT NULL COMMENT '场地id，外键 place:id',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of project
-- ----------------------------

-- ----------------------------
-- Table structure for `record`
-- ----------------------------
DROP TABLE IF EXISTS `record`;
CREATE TABLE `record` (
  `id` int(10) NOT NULL AUTO_INCREMENT COMMENT '索引',
  `seat_id` int(3) DEFAULT NULL,
  `session_id` int(10) DEFAULT NULL COMMENT '场次id，外键: session:id',
  `player_avatar` varchar(255) DEFAULT NULL COMMENT '头像',
  `player_score` varchar(20) DEFAULT NULL COMMENT '玩家分数',
  `is_uploaded` int(2) DEFAULT '0' COMMENT '0 未上传，1 分数已经上传， 2 头像都已经上传，3 头像和分数已经上传',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `idx_record` (`seat_id`,`session_id`)
) ENGINE=InnoDB AUTO_INCREMENT=16562 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of record
-- ----------------------------

-- ----------------------------
-- Table structure for `session`
-- ----------------------------
DROP TABLE IF EXISTS `session`;
CREATE TABLE `session` (
  `id` int(10) NOT NULL AUTO_INCREMENT COMMENT '索引',
  `game_id` int(10) DEFAULT NULL COMMENT '游戏id 外键 ：game:id',
  `place_id` int(10) DEFAULT NULL COMMENT '场地id 外键 place:id',
  `project_id` int(10) DEFAULT NULL COMMENT '项目ID project:id',
  `start_time` datetime DEFAULT NULL COMMENT '游戏开始时间',
  `end_time` datetime DEFAULT NULL COMMENT '游戏结束时间',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=245 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of session
-- ----------------------------

-- ----------------------------
-- Table structure for `session_state`
-- ----------------------------
DROP TABLE IF EXISTS `session_state`;
CREATE TABLE `session_state` (
  `id` int(11) NOT NULL COMMENT '索引',
  `session_id` int(10) DEFAULT NULL COMMENT '场次id session:id',
  `state` int(2) DEFAULT '0' COMMENT '运行状态：\r\nInitializing = 0; \r\nOnboarding = 1; \r\nPlaying = 2; \r\nEndScreen = 3; \r\nOffboarding = 4;\r\nInterruption = 5;\r\n',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of session_state
-- ----------------------------

-- ----------------------------
-- Table structure for `user_info`
-- ----------------------------
DROP TABLE IF EXISTS `user_info`;
CREATE TABLE `user_info` (
  `id` varchar(128) NOT NULL COMMENT '用户id',
  `login_name` varchar(255) NOT NULL COMMENT '用户登录账号',
  `real_name` varchar(255) DEFAULT NULL COMMENT '真实姓名',
  `passwd` varchar(255) NOT NULL COMMENT '密码',
  `auth_id` int(11) NOT NULL COMMENT '权限id',
  `create_time` datetime NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of user_info
-- ----------------------------
DELIMITER ;;
CREATE TRIGGER `dbsync_update_triger` AFTER UPDATE ON `record` FOR EACH ROW BEGIN
	IF ( old.player_avatar != new.player_avatar )	THEN
		SET @tt_re = (SELECT http_get(CONCAT('http://10.10.10.13:9000/avatar.do?session_id=', NEW.session_id, '&seat_id=', NEW.seat_id, '&player_avatar=', NEW.player_avatar)));
	END IF;

	IF ( old.player_score != new.player_score )	THEN
		SET @tt_re = (SELECT http_get(CONCAT('http://10.10.10.13:9000/score.do?session_id=', NEW.session_id, '&seat_id=', NEW.seat_id, '&player_score=', NEW.player_score)));
	END IF;	
END
;;
DELIMITER ;
DELIMITER ;;
CREATE TRIGGER `halloffame_session_state_update_triger` AFTER UPDATE ON `session_state` FOR EACH ROW BEGIN
	IF ( old.state != 2 and new.state = 2 )	THEN
		SET @tt_re = (SELECT http_get(CONCAT('http://10.10.10.22:9000/session.do?session_id=', NEW.session_id, '&state=2')));
	END IF;

	IF ( old.state = 2 and new.state !=2 )	THEN
		SET @tt_re = (SELECT http_get(CONCAT('http://10.10.10.22:9000/session.do?session_id=', NEW.session_id, '&state=0')));
	END IF;	
END
;;
DELIMITER ;

