<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Analysis extends CI_Controller {

    public function index() {
        redirect('analysis/current');
    }

    public function current() {
        $this->influxdb->setTimePrecision('ms');
        $db = $this->influxdb->getDatabase();

        $data['current'] = new stdClass();
        $res = $db->query("SELECT time, MEAN(value) FROM 10c.current.rms where time > " . strtotime('today') . " GROUP BY time(1m);");
        $data['current']->one_minute = $res['10c.current.rms'];

        $res = $db->query("SELECT MEAN(value), MAX(value), MIN(value) FROM 10c.current.rms where time > " . strtotime('today') . "s;");
        if (count($res) > 0) {
            $data['current']->today = $res['10c.current.rms'][0]->mean;
            $data['current']->max_today = $res['10c.current.rms'][0]->max;
            $data['current']->min_today = $res['10c.current.rms'][0]->min;
        } else {
            $data['current']->today = 0;
            $data['current']->max_today = 0;
            $data['current']->min_today = 0;
        }


        $this->load->model('data_model');
        $data['main_values'] = $this->data_model->getMainValues();

        $this->load->view('analysis/current_view', $data);
    }

    public function ajaxGetVoltageWaveform() {
        if ($this->input->is_ajax_request()) {
            $snd_msg = "OSCILLOSCOPE_V|" . $this->input->post('lastTimestamp');
            $rcv_msg = "";
            $timeout = 5;
            $socket = stream_socket_client('unix:///var/run/openZmeter.sock', $errorno, $errorstr, $timeout);
            stream_set_timeout($socket, $timeout);

            //echo("Sending Message...<br>\n");
            if (!fwrite($socket, $snd_msg)) {
                echo "-1";
                return;
            }
            //echo("Error while writing!!!<br>\n");
            //echo("Receiving Message...<br>\n");
            do {
                $data = @fread($socket, 8192);
                if (strlen($data) == 0) {
                    break;
                }
                $rcv_msg .= $data;
            } while (true);
            //$points = json_decode($rcv_msg);
            //print_r($points);
            echo $rcv_msg;
        }
    }

    public function ajaxGetVoltageHarmonics() {
        if ($this->input->is_ajax_request()) {
            $this->influxdb->setTimePrecision('ms');
            $db = $this->influxdb->getDatabase();

            $res = $db->query("SELECT * FROM 10c.voltage.harm" /* WHERE time > now() - 10s */ . "  LIMIT 1;");
            if (!isset($res['10c.voltage.harm'])) {
                $harmonics = -1;
            } else {
                $harmonics = $res['10c.voltage.harm'];
            }

            echo json_encode($harmonics);
        }
    }

    public function ajaxGetCurrentWaveform() {
        if ($this->input->is_ajax_request()) {
            $snd_msg = "OSCILLOSCOPE_I|" . $this->input->post('lastTimestamp');
            $rcv_msg = "";
            $timeout = 5;
            $socket = stream_socket_client('unix:///var/run/openZmeter.sock', $errorno, $errorstr, $timeout);
            stream_set_timeout($socket, $timeout);

            //echo("Sending Message...<br>\n");
            if (!fwrite($socket, $snd_msg)) {
                echo "-1";
            }
            //echo("Error while writing!!!<br>\n");
            //echo("Receiving Message...<br>\n");
            do {
                $data = @fread($socket, 8192);
                if (strlen($data) == 0) {
                    break;
                }
                $rcv_msg .= $data;
            } while (true);
            //$points = json_decode($rcv_msg);
            //print_r($points);
            echo $rcv_msg;
        }
    }

    public function ajaxGetCurrentHarmonics() {
        if ($this->input->is_ajax_request()) {
            $this->influxdb->setTimePrecision('ms');
            $db = $this->influxdb->getDatabase();

            $res = $db->query("SELECT * FROM 10c.current.harm  "/* WHERE time > now() - 1s */ . "LIMIT 1;");
            if (!isset($res['10c.current.harm'])) {
                $harmonics = -1;
            } else {
                $harmonics = $res['10c.current.harm'];
            }

            echo json_encode($harmonics);
        }
    }

}
