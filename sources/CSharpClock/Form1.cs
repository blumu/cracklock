using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace Clock
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }
        private void RefreshDateTime()
        {
            labTime.Text = DateTime.Now.ToString();
            if( System.TimeZone.CurrentTimeZone.IsDaylightSavingTime(DateTime.Now) )
                labTz.Text = System.TimeZone.CurrentTimeZone.StandardName;
            else
                labTz.Text = System.TimeZone.CurrentTimeZone.DaylightName;
        }
        private void timer_Tick(object sender, EventArgs e)
        {
            RefreshDateTime();
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void label2_Click(object sender, EventArgs e)
        {

        }

        private void Form1_Load(object sender, EventArgs e)
        {
            RefreshDateTime();
        }
    }
}