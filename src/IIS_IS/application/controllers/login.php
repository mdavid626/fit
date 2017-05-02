<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Description of Login
 * Controller for controlling login operations
 * @author xmolna02
 */
class Login extends CI_Controller {
    
    var $flag;
    
    function __construct()
    {
        parent::__construct();
        $this->flag = 0;
    }
    
    public function index()
    {      
		if($this->session->userdata('role') != M_UNAUTH)
		{
            redirect('site');
		}   
        
        $data['main_content'] = 'login_form';
        $data['role'] = M_UNAUTH;
        $data['page_title'] = 'Prihlásenie';
        $data['flag'] = $this->flag;
        
        $this->load->view('includes/template', $data);
    }
    
    public function logged_out()
    {
		if($this->session->userdata('role') != M_UNAUTH)
		{
            redirect('site');
		}   
        
        $data['main_content'] = 'successful_logout_view';
        $data['role'] = M_UNAUTH;
        $data['page_title'] = 'Prihlásenie';
        $data['flag'] = $this->flag;
        
        $this->load->view('includes/template', $data);
    }
    
    function validate()
    {
        $this->load->model('user_model');
        $query = $this->user_model->validate();
        
        if($query['role'] != M_UNAUTH) // if the user's credentials validated...
        {
            $data = array(
                'username' => $query['username'],
                'fname' => $query['fname'],
                'role' => $query['role'],
                'r_cislo' => $query['r_cislo']
            );
            
            $this->session->set_userdata($data);
            redirect('site');
        }
        else // incorrect username or password
        {
            $this->flag = 1;
            $this->index();     
        }
    }
    
    function create_member()
    {
        // not implemented
    }
    
    function logout()
    {
        $this->session->sess_destroy();
        redirect('login/logged_out');
    }
}

/* End of file login.php */
/* Location: ./application/controllers/login.php */