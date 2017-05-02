<?php

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

//put your code here
function format_date($date)
{
    if ($date)
    {
        return date('d.m.Y H:i', strtotime($date));
    }
    else
    {
        return '';
    }
}

function sk_to_date($str)
{
    return preg_replace("/^(\d{1,2})[\/\. -]+(\d{1,2})[\/\. -]+(\d{1,4}) (\d{1,2}:\d{1,2})/", "\\3-\\2-\\1 \\4", $str);
}


?>
