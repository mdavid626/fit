<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
require_once 'includes/authtool.php';

/**
 * Description of Site
 * Controller for controlling main page
 * @author xmolna02
 */
class Site extends CI_Controller {

    public function index()
    {
        $data = array();
        AuthTool::load_template('home_view', 'Hlavná stránka', $data);
    }
    
}

/* End of file site.php */
/* Location: ./application/controllers/site.php */