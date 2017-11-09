<?php
defined('BASEPATH') OR exit('No direct script access allowed');
class Settings extends CI_Controller {
  public function index() {
    $data = array();
    $data['settings'] = parse_ini_file("openZmeter.ini", true);
    $this->load->model('data_model');
    $data['main_values'] = $this->data_model->getMainValues();
    $this->load->model('event_model');
    $data['numEvents'] = $this->event_model->getNumUnreadEvents($this->session->eventTimestamp);
    $this->load->view('settings_view', $data);
  }
  public function ajaxSaveSettings() {
    if($this->input->is_ajax_request()) {
      $config = array(
          'database' => $this->input->post('database'),
          'electrical' => $this->input->post('electrical'),
          'budget' => $this->input->post('budget')
      );
      if ($config['budget']['fixed_price'] == "false") {
        $config['budget-' . $config['budget']['country']] = $this->input->post('budget_country');
      }
      $this->load->library('IniWriter');
      $result = true;
      try {
        $this->iniwriter->writeToFile('openzmeter.ini', $config, '; openZmeter config file | Generated on the fly via web at ' . date('l jS \of F Y h:i:s A') . "\n\n");
      } catch (Exception $e) {
        $result = false;
      }
      echo json_encode(array('result' => $result));
    }
  }

}
