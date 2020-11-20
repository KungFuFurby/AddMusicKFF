using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace AMKGUI
{
	public partial class ResultForm : Form
	{
		public ResultForm(string results, bool success)
		{
			InitializeComponent();

			textBox1.Text = results;

			if (success)
			{
				label1.Text = @"This is the output from AddmusicK. It appears that there were no errors, so your ROM should be all set!

When you are finished, just close this window.";
			}
			else
			{
				label1.Text = @"This is the output from AddmusicK. There was an error somewhere. Review this information to find the problem, and please see the readme if you need help!

When you are finished, just close this window.";
			}

			textBox1.Focus();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			Close();
		}
	}
}
