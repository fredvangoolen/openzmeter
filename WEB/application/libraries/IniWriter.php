<?php

if (!defined('BASEPATH'))
    exit('No direct script access allowed');

require_once dirname(__FILE__) . '/component-ini/IniWritingException.php';
require_once dirname(__FILE__) . '/component-ini/IniWriter.php';

class IniWriter extends Piwik\Ini\IniWriter {

    public function __construct() {
        parent::__construct();
    }

}
