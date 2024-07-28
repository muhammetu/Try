using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;

using System.Windows.Forms;
using Config.Utils;

namespace Config
{
    public partial class MainForm : Form
    {
        private readonly ConfigFile _conf;

        private bool _ready;

        public MainForm()
        {
            InitializeComponent();
    
            // Load Configurations
            _conf = new ConfigFile(Path.Combine(Environment.CurrentDirectory, "phebia.cfg"));
        }


        private void Form1Load(object sender, EventArgs e)
        {



            Icon = Properties.Resources.metin2;
            // Resolution
            List<Resolution> resolutions = Win32.EnumAvailableResolutions();
    
            var res = _conf.GetResolution();

            if (!resolutions.Contains(res))
                resolutions.Add(res);
            comboResolution.DataSource = resolutions;
            comboResolution.SelectedItem = res;
#if !DEBUG
            try
            {
#endif
                // Fullscreen
                checkWindowed.Checked = _conf.Read( "WINDOWED", true);

            // Fog
            Utils.Utils.SafeSetIndex(comboFog, _conf.Read("VISIBILITY", 3) - 1);


            // Shadows
            Utils.Utils.SafeSetIndex(comboShadows, _conf.Read("SHADOW_LEVEL", 3));
       

                // Music volume
                trackMusic.Value = (int) (_conf.Read("MUSIC_VOLUME", 1.0f)*100);
                // SFX Volume
                trackSFX.Value = _conf.Read("VOICE_VOLUME", 5);
                // Show damage
                checkShowDmg.Checked = _conf.Read("SHOW_DAMAGE", true);
                // View Chat
                checkShowChat.Checked = _conf.Read("VIEW_CHAT", true);
#if !DEBUG
            }
            catch (Exception ex)
            {
                if (
                    MessageBox.Show("Looks like the configuration file is corrupted. Do you want to delete it?", "Error",
                        MessageBoxButtons.YesNo) == DialogResult.Yes)
                {
                    MessageBox.Show("Reopen the configuration.");
                    File.Delete("phebia.cfg");
                    Environment.Exit(0);
                }
            }
#endif

            _ready = true;
        }

        private void ComboResolutionSelectedIndexChanged(object sender, EventArgs e)
        {
            if (_ready)
                _conf.SetResolution((Resolution) comboResolution.SelectedItem);
        }

        private void ComboFogSelectedIndexChanged(object sender, EventArgs e)
        {
            if (_ready)
                _conf.Write("VISIBILITY", comboFog.SelectedIndex+1);
        }

        private void ComboShadowsSelectedIndexChanged(object sender, EventArgs e)
        {
            if (_ready)
                _conf.Write("SHADOW_LEVEL", comboShadows.SelectedIndex );
        }

        private void TrackSfxScroll(object sender, EventArgs e)
        {
            if (_ready)
                _conf.Write("VOICE_VOLUME", trackSFX.Value);
        }

        private void TrackMusicScroll(object sender, EventArgs e)
        {
            if (_ready)
                _conf.Write("MUSIC_VOLUME", (float)trackMusic.Value / (float)100);
        }

        private void CheckFullscreenCheckedChanged(object sender, EventArgs e)
        {
            if (_ready)
                _conf.Write("WINDOWED", checkWindowed.Checked);
        }



        private void CheckShowDmgCheckedChanged(object sender, EventArgs e)
        {
            if (_ready)
                _conf.Write("SHOW_DAMAGE", checkShowDmg.Checked);
        }

        private void CheckShowChatCheckedChanged(object sender, EventArgs e)
        {
            if (_ready)
                _conf.Write("VIEW_CHAT", checkShowChat.Checked);
        }


        private void OnFormClosing(object sender, FormClosingEventArgs e)
        {
            _conf.Save();
        }

        private void labelTNL_Click(object sender, EventArgs e)
        {

        }

        private void audioContainer_Enter(object sender, EventArgs e)
        {

        }
    }
}
