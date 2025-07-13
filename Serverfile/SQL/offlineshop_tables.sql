/*
Navicat MySQL Data Transfer

Source Server         : GoogleCloud - T-Server
Source Server Version : 50649
Source Host           : 34.65.186.129:3306
Source Database       : player

Target Server Type    : MYSQL
Target Server Version : 50649
File Encoding         : 65001

Date: 2020-10-14 19:33:20
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for offlineshop_auction_offers
-- ----------------------------
DROP TABLE IF EXISTS `offlineshop_auction_offers`;
CREATE TABLE `offlineshop_auction_offers` (
  `owner_id` bigint(20) unsigned NOT NULL,
  `buyer_id` bigint(20) unsigned NOT NULL,
  `buyer_name` varchar(255) NOT NULL,
  `yang` bigint(20) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for offlineshop_auctions
-- ----------------------------
DROP TABLE IF EXISTS `offlineshop_auctions`;
CREATE TABLE `offlineshop_auctions` (
  `owner_id` bigint(20) unsigned NOT NULL,
  `duration` bigint(20) unsigned NOT NULL DEFAULT '0',
  `init_yang` bigint(255) unsigned DEFAULT NULL,
  `name` varchar(255) NOT NULL,
  `vnum` bigint(20) unsigned NOT NULL,
  `count` int(10) unsigned NOT NULL,
  `socket0` int(10) unsigned NOT NULL,
  `socket1` int(10) unsigned NOT NULL,
  `socket2` int(10) unsigned NOT NULL,
  `attr0` int(10) unsigned NOT NULL,
  `attrval0` int(10) NOT NULL,
  `attr1` int(10) unsigned NOT NULL,
  `attrval1` int(10) NOT NULL,
  `attr2` int(10) unsigned NOT NULL,
  `attrval2` int(10) NOT NULL,
  `attr3` int(10) unsigned NOT NULL,
  `attrval3` int(10) NOT NULL,
  `attr4` int(10) unsigned NOT NULL,
  `attrval4` int(10) NOT NULL,
  `attr5` int(10) unsigned NOT NULL,
  `attrval5` int(10) NOT NULL,
  `attr6` int(10) unsigned NOT NULL,
  `attrval6` int(10) NOT NULL,
  `expiration` tinyint(11) DEFAULT '0',
  PRIMARY KEY (`owner_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for offlineshop_logs
-- ----------------------------
DROP TABLE IF EXISTS `offlineshop_logs`;
CREATE TABLE `offlineshop_logs` (
  `owner_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `what` varchar(500) CHARACTER SET utf8mb4 NOT NULL,
  `when` datetime(6) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for offlineshop_offers
-- ----------------------------
DROP TABLE IF EXISTS `offlineshop_offers`;
CREATE TABLE `offlineshop_offers` (
  `offer_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `owner_id` bigint(20) NOT NULL,
  `offerer_id` bigint(20) NOT NULL,
  `item_id` bigint(20) NOT NULL,
  `price_yang` bigint(20) NOT NULL,
  `price_cheque` bigint(20) unsigned NOT NULL DEFAULT '0',
  `is_notified` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `is_accept` tinyint(3) unsigned NOT NULL,
  `buyer_name` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`offer_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for offlineshop_safebox_items
-- ----------------------------
DROP TABLE IF EXISTS `offlineshop_safebox_items`;
CREATE TABLE `offlineshop_safebox_items` (
  `item_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `owner_id` bigint(20) unsigned NOT NULL,
  `vnum` bigint(20) unsigned NOT NULL,
  `count` int(10) unsigned NOT NULL,
  `socket0` int(10) unsigned NOT NULL,
  `socket1` int(10) unsigned NOT NULL,
  `socket2` int(10) unsigned NOT NULL,
  `attr0` int(10) unsigned NOT NULL,
  `attrval0` int(10) NOT NULL,
  `attr1` int(10) unsigned NOT NULL,
  `attrval1` int(10) NOT NULL,
  `attr2` int(10) unsigned NOT NULL,
  `attrval2` int(10) NOT NULL,
  `attr3` int(10) unsigned NOT NULL,
  `attrval3` int(10) NOT NULL,
  `attr4` int(10) unsigned NOT NULL,
  `attrval4` int(10) NOT NULL,
  `attr5` int(10) unsigned NOT NULL,
  `attrval5` int(10) NOT NULL,
  `attr6` int(10) unsigned NOT NULL,
  `attrval6` int(10) NOT NULL,
  `expiration` tinyint(11) DEFAULT '0',
  PRIMARY KEY (`item_id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=21869 DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for offlineshop_safebox_valutes
-- ----------------------------
DROP TABLE IF EXISTS `offlineshop_safebox_valutes`;
CREATE TABLE `offlineshop_safebox_valutes` (
  `owner_id` bigint(20) unsigned NOT NULL,
  `yang` bigint(20) unsigned NOT NULL DEFAULT '0',
  `cheque` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`owner_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for offlineshop_shop_items
-- ----------------------------
DROP TABLE IF EXISTS `offlineshop_shop_items`;
CREATE TABLE `offlineshop_shop_items` (
  `item_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `owner_id` bigint(20) unsigned NOT NULL,
  `price_yang` bigint(20) unsigned NOT NULL DEFAULT '0',
  `price_cheque` int(10) unsigned NOT NULL DEFAULT '0',
  `vnum` bigint(20) unsigned NOT NULL,
  `count` int(10) unsigned NOT NULL,
  `socket0` int(10) unsigned NOT NULL,
  `socket1` int(10) unsigned NOT NULL,
  `socket2` int(10) unsigned NOT NULL,
  `attr0` int(10) unsigned NOT NULL,
  `attrval0` int(10) NOT NULL,
  `attr1` int(10) unsigned NOT NULL,
  `attrval1` int(10) NOT NULL,
  `attr2` int(10) unsigned NOT NULL,
  `attrval2` int(10) NOT NULL,
  `attr3` int(10) unsigned NOT NULL,
  `attrval3` int(10) NOT NULL,
  `attr4` int(10) unsigned NOT NULL,
  `attrval4` int(10) NOT NULL,
  `attr5` int(10) unsigned NOT NULL,
  `attrval5` int(10) NOT NULL,
  `attr6` int(10) unsigned NOT NULL,
  `attrval6` int(10) NOT NULL,
  `is_sold` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `expiration` tinyint(11) DEFAULT '0',
  PRIMARY KEY (`item_id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Table structure for offlineshop_shops
-- ----------------------------
DROP TABLE IF EXISTS `offlineshop_shops`;
CREATE TABLE `offlineshop_shops` (
  `owner_id` bigint(20) unsigned NOT NULL,
  `name` varchar(255) NOT NULL,
  `duration` int(10) unsigned NOT NULL,
  `pos_x` int(10) NOT NULL,
  `pos_y` int(10) NOT NULL,
  `map_index` int(10) NOT NULL,
  `channel` int(10) NOT NULL,
  PRIMARY KEY (`owner_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;
