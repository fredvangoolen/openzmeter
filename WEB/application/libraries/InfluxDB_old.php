<?php

if (!defined('BASEPATH'))
    exit('No direct script access allowed');

require_once dirname(__FILE__) . '/InfluxPHP/BaseHTTP.php';
require_once dirname(__FILE__) . '/InfluxPHP/Client.php';
require_once dirname(__FILE__) . '/InfluxPHP/Cursor.php';
require_once dirname(__FILE__) . '/InfluxPHP/DB.php';

class InfluxDB extends crodas\InfluxPHP\Client {

    public function __construct() {
        //$CI = & get_instance();
        //$CI->load->config('InfluxDB');
        //parent::__construct($CI->config->item('idb_host'), $CI->config->item('idb_port'), $CI->config->item('idb_user'), $CI->config->item('idb_pass'), $CI->config->item('idb_database'));

        $settings = parse_ini_file("openzmeter.ini", true);
        parent::__construct($settings['database']['host'], $settings['database']['port'], $settings['database']['user'], $settings['database']['pass'], $settings['database']['database']);
    }

}
