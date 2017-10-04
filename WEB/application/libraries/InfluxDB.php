<?php

if (!defined('BASEPATH'))
    exit('No direct script access allowed');

require_once dirname(__FILE__) . '/InfluxPHP-0.9.0/BaseHTTP.php';
require_once dirname(__FILE__) . '/InfluxPHP-0.9.0/Client.php';
require_once dirname(__FILE__) . '/InfluxPHP-0.9.0/DB.php';
require_once dirname(__FILE__) . '/InfluxPHP-0.9.0/MultipleResultSeriesObject.php';
require_once dirname(__FILE__) . '/InfluxPHP-0.9.0/ResultSeriesObject.php';
require_once dirname(__FILE__) . '/InfluxPHP-0.9.0/ResultsetBuilder.php';

class InfluxDB extends crodas\InfluxPHP\Client {

    public function __construct() {
//        $CI = & get_instance();
//        $CI->load->config('InfluxDB');
//        parent::__construct($CI->config->item('idb_host'), $CI->config->item('idb_port'), $CI->config->item('idb_user'), $CI->config->item('idb_pass'));
        $settings = parse_ini_file("openzmeter.ini", true);
        parent::__construct($settings['database']['host'], $settings['database']['port'], $settings['database']['user'], $settings['database']['pass'], $settings['database']['database']);
    }

}
